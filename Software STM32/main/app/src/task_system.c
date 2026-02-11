/* Project includes. */
#include "main.h"
#include <string.h>

/* Demo includes. */
#include "logger.h"

/* Application & Tasks includes. */
#include "app.h"

#if APP_TEST_MODE
#define TEST_LOG(...) LOGGER_LOG(__VA_ARGS__)
#else
#define TEST_LOG(...)
#endif

/********************** macros and definitions *******************************/
#define FLASH_MAGIC_WORD             0xA5A55A5Au
#define FLASH_LIGHT_STATE_ADDR       0x0801FC00u
#define FLASH_LIGHT_MAGIC_ADDR       FLASH_LIGHT_STATE_ADDR
#define FLASH_LIGHT_VALUE_ADDR       (FLASH_LIGHT_STATE_ADDR + 4u)
#define FAULT_RECOVERY_MS            10000u
#define FAULT_BLINK_MS               1000u

typedef enum {
    ST_INIT_READ_FLASH = 0,
    ST_INIT_READ_DIP,
    ST_INIT_CHECK_SENSORS,
    ST_INIT_RESTORE_PWM,
    ST_INIT_CONFIG_BT,
    ST_NORMAL,
    ST_FAULT
} system_state_t;

/********************** internal functions declaration ***********************/
static bool flash_load_light_state(bool *light_on);
static bool flash_store_light_state(bool light_on);
static uint16_t build_fan_delay_from_adc(uint16_t adc_raw);
static void apply_dip_roles(shared_data_type *shared_data);
static bool bt_probe_hc06_at(void);

/********************** internal data definition *****************************/
const char *p_task_system = "Task System";

/********************** external data declaration *****************************/
extern UART_HandleTypeDef huart1;

/********************** external functions definition ************************/
void task_system_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *)parameters;

    TEST_LOG("  %s is running - %s\r\n", GET_NAME(task_system_init), p_task_system);

    shared_data->fault_mode = false;
    shared_data->cut_off_voltage = false;
    shared_data->alarm_on = false;
    shared_data->flash_save_light_request = false;
}

void task_system_update(void *parameters)
{
    static system_state_t state = ST_INIT_READ_FLASH;
    static system_state_t last_state = ST_FAULT;
    static uint32_t fault_elapsed_ms = 0u;
    static uint32_t fault_blink_ms = 0u;
    static bool fault_led_on = false;

    shared_data_type *shared_data = (shared_data_type *)parameters;
    bool ok = true;

    if (state != last_state) {
        TEST_LOG("[SYS] state -> %lu t=%lu ms\r\n", (uint32_t)state, HAL_GetTick());
        last_state = state;
    }

    /* FSM principal del sistema: Init -> Normal o Fault. */
    switch (state) {
    case ST_INIT_READ_FLASH:
        /* Etapa 1: restaura el estado de luz desde flash. */
        ok = flash_load_light_state(&shared_data->light_enabled);
        if (ok) {
            state = ST_INIT_READ_DIP;
        } else {
            fault_elapsed_ms = 0u;
            fault_blink_ms = 0u;
            fault_led_on = true;
            shared_data->alarm_on = true;
            state = ST_FAULT;
        }
        break;

    case ST_INIT_READ_DIP:
        /* Etapa 2: aplica roles de los DIP (BT/Buzzer/LED/Reservado). */
        apply_dip_roles(shared_data);
        state = ST_INIT_CHECK_SENSORS;
        break;

    case ST_INIT_CHECK_SENSORS:
        /* Etapa 3: placeholder para chequeos de sensores/periféricos. */
        state = ST_INIT_RESTORE_PWM;
        break;

    case ST_INIT_RESTORE_PWM:
        /* Etapa 4: reconstruye delay del fan a partir del ADC actual. */
        shared_data->fan_delay_us = build_fan_delay_from_adc(shared_data->adc_raw);
        state = ST_INIT_CONFIG_BT;
        break;

    case ST_INIT_CONFIG_BT:
        /* Etapa 5: configuración lógica final y entrada a modo normal. */
        apply_dip_roles(shared_data);
#if APP_BT_AT_PROBE_ON_INIT
        if (shared_data->bt_enabled) {
            bool bt_ok = bt_probe_hc06_at();
            TEST_LOG("[SYS] BT probe AT -> %s\r\n", bt_ok ? "OK" : "FAIL");
#if APP_BT_AT_PROBE_STRICT
            if (!bt_ok) {
                fault_elapsed_ms = 0u;
                fault_blink_ms = 0u;
                fault_led_on = true;
                shared_data->alarm_on = true;
                state = ST_FAULT;
                break;
            }
#endif
        }
#endif
        state = ST_NORMAL;
        shared_data->fault_mode = false;
        shared_data->cut_off_voltage = false;
        shared_data->alarm_on = false;
        break;

    case ST_NORMAL:
        /* Modo operativo normal: procesa eventos limpios de sensor. */
        apply_dip_roles(shared_data);

#if APP_TEST_MODE
        /* Prueba: DIP4 en 1 fuerza entrada a estado Fault. */
        if ((shared_data->dip_value & (1u << 3)) != 0u) {
            fault_elapsed_ms = 0u;
            fault_blink_ms = 0u;
            fault_led_on = true;
            shared_data->alarm_on = true;
            TEST_LOG("[SYS] test: DIP4=1 -> force ST_FAULT\r\n");
            state = ST_FAULT;
            break;
        }
#endif

        if (shared_data->ev_sys_pressed) {
            shared_data->ev_sys_pressed = false;
            shared_data->light_enabled = !shared_data->light_enabled;
            shared_data->ev_send_bt_update_light = true;
            shared_data->flash_save_light_request = true;
            TEST_LOG("[SYS] toggle light=%u t=%lu ms\r\n",
                     shared_data->light_enabled ? 1u : 0u,
                     HAL_GetTick());
        }

        if (shared_data->ev_pote_changed) {
            shared_data->ev_pote_changed = false;
            shared_data->ev_send_bt_update_pote = true;
            TEST_LOG("[SYS] pote changed adc=%u%% delay=%u us\r\n",
                     shared_data->adc_percent,
                     shared_data->fan_delay_us);
        }

        if (shared_data->flash_save_light_request) {
            shared_data->flash_save_light_request = false;
            if (!flash_store_light_state(shared_data->light_enabled)) {
                /* Si falla persistencia crítica, pasa a modo Fault. */
                fault_elapsed_ms = 0u;
                fault_blink_ms = 0u;
                fault_led_on = true;
                shared_data->alarm_on = true;
                state = ST_FAULT;
            } else {
                TEST_LOG("[SYS] flash save light=%u OK\r\n",
                         shared_data->light_enabled ? 1u : 0u);
            }
        }
        break;

    case ST_FAULT:
        /* Modo seguro: corta potencia y parpadea alarma 1 Hz. */
        apply_dip_roles(shared_data);
        shared_data->fault_mode = true;
        shared_data->cut_off_voltage = true;

        fault_elapsed_ms++;
        fault_blink_ms++;
        if (fault_blink_ms >= FAULT_BLINK_MS) {
            fault_blink_ms = 0u;
            fault_led_on = !fault_led_on;
            shared_data->alarm_on = fault_led_on;
        }

        if (fault_elapsed_ms >= FAULT_RECOVERY_MS) {
            /* Watchdog lógico: reintenta inicialización cada 10 s. */
            fault_elapsed_ms = 0u;
            fault_blink_ms = 0u;
            state = ST_INIT_READ_FLASH;
        }
        break;

    default:
        state = ST_INIT_READ_FLASH;
        break;
    }
}

/********************** internal functions definition ************************/
static bool flash_load_light_state(bool *light_on)
{
    uint32_t magic = *(volatile uint32_t *)FLASH_LIGHT_MAGIC_ADDR;
    uint32_t value = *(volatile uint32_t *)FLASH_LIGHT_VALUE_ADDR;

    if (magic == FLASH_MAGIC_WORD) {
        *light_on = ((value & 0x1u) != 0u);
    } else {
        *light_on = false;
    }

    return true;
}

static bool flash_store_light_state(bool light_on)
{
    HAL_StatusTypeDef hal_status;
    FLASH_EraseInitTypeDef erase_cfg;
    uint32_t page_error = 0u;

    /* Borra y reescribe una página dedicada con magic + estado de luz. */
    hal_status = HAL_FLASH_Unlock();
    if (hal_status != HAL_OK) {
        return false;
    }

    erase_cfg.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_cfg.PageAddress = FLASH_LIGHT_STATE_ADDR;
    erase_cfg.NbPages = 1u;
    hal_status = HAL_FLASHEx_Erase(&erase_cfg, &page_error);
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_LIGHT_MAGIC_ADDR, FLASH_MAGIC_WORD);
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_LIGHT_VALUE_ADDR, (uint32_t)(light_on ? 1u : 0u));
    (void)HAL_FLASH_Lock();

    return (hal_status == HAL_OK);
}

static uint16_t build_fan_delay_from_adc(uint16_t adc_raw)
{
    return (uint16_t)((adc_raw * 7500u) / 4095u);
}

static void apply_dip_roles(shared_data_type *shared_data)
{
    /* DIP role map:
     * DIP1 -> Bluetooth enable
     * DIP2 -> Buzzer enable
     * DIP3 -> LED enable
     * DIP4 -> Reserved / currently unused
     */
    shared_data->bt_enabled = ((shared_data->dip_value & (1u << 0)) != 0u);
    shared_data->buzzer_enabled = ((shared_data->dip_value & (1u << 1)) != 0u);
    shared_data->led_enabled = ((shared_data->dip_value & (1u << 2)) != 0u);
}

static bool bt_probe_hc06_at(void)
{
    static const uint8_t cmd[] = "AT";
    uint8_t ch = 0u;
    uint8_t rx[32];
    uint16_t idx = 0u;
    uint32_t t0 = HAL_GetTick();

    /* Limpia posible basura previa en el RX. */
    while (HAL_UART_Receive(&huart1, &ch, 1u, 1u) == HAL_OK) {
    }

    if (HAL_UART_Transmit(&huart1, (uint8_t *)cmd, sizeof(cmd) - 1u, 50u) != HAL_OK) {
        return false;
    }

    while ((HAL_GetTick() - t0) < 250u) {
        if (HAL_UART_Receive(&huart1, &ch, 1u, 10u) == HAL_OK) {
            if (idx < (sizeof(rx) - 1u)) {
                rx[idx++] = ch;
                rx[idx] = '\0';
            }

            if (strstr((const char *)rx, "OK") != NULL) {
                return true;
            }
        }
    }

    return false;
}

/********************** end of file ******************************************/
