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

#if APP_TEST_MODE && APP_TEST_SYS_VERBOSE_LOGS
#define TEST_LOG_SYS(...) LOGGER_LOG(__VA_ARGS__)
#else
#define TEST_LOG_SYS(...)
#endif

/********************** macros and definitions *******************************/
#define FLASH_MAGIC_WORD             0xA5A55A5Au
#define FLASH_LAYOUT_VERSION         0x00000002u
#define FLASH_STATE_PAGE_ADDR        0x0801FC00u
#define FLASH_STATE_MAGIC_ADDR       FLASH_STATE_PAGE_ADDR
#define FLASH_STATE_META_ADDR        (FLASH_STATE_PAGE_ADDR + 4u)
#define FLASH_STATE_LIGHT_ADDR       (FLASH_STATE_PAGE_ADDR + 8u)
#define FLASH_STATE_ADC_MIN_ADDR     (FLASH_STATE_PAGE_ADDR + 12u)
#define FLASH_STATE_ADC_MAX_ADDR     (FLASH_STATE_PAGE_ADDR + 16u)
#define FLASH_ADC_INVALID_WORD       0xFFFFFFFFu
#define ADC_FULL_SCALE_COUNTS        4095u
#define FAULT_RECOVERY_MS            10000u
#define FAULT_BLINK_MS               500u

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
static bool flash_load_persistent_state(shared_data_type *shared_data);
static bool flash_store_persistent_state(bool light_on,
                                         bool adc_calib_valid,
                                         uint16_t adc_calib_min,
                                         uint16_t adc_calib_max);
static uint16_t build_fan_delay_from_percent(uint8_t adc_percent);
static void apply_dip_roles(shared_data_type *shared_data);
static bool bt_probe_hc06_at(void);
static bool dip_fault_forced(const shared_data_type *shared_data);

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
    shared_data->flash_save_adc_calib_request = false;
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
        TEST_LOG_SYS("[SYS] state -> %lu t=%lu ms\r\n", (uint32_t)state, HAL_GetTick());
        last_state = state;
    }

    /* FSM principal del sistema: Init -> Normal o Fault. */
    switch (state) {
    case ST_INIT_READ_FLASH:
        /* Etapa 1: restaura estado de luz y calibración ADC desde flash. */
        ok = flash_load_persistent_state(shared_data);
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
        /* Etapa 4: reconstruye delay del fan a partir del porcentaje ya escalado. */
        shared_data->fan_delay_us = build_fan_delay_from_percent(shared_data->adc_percent);
        state = ST_INIT_CONFIG_BT;
        break;

    case ST_INIT_CONFIG_BT:
        /* Etapa 5: configuración lógica final y entrada a modo normal. */
        apply_dip_roles(shared_data);
#if APP_BT_AT_PROBE_ON_INIT
        if (shared_data->bt_enabled) {
            bool bt_ok = bt_probe_hc06_at();
            TEST_LOG_SYS("[SYS] BT probe AT -> %s\r\n", bt_ok ? "OK" : "FAIL");
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

        /* DIP4 en 1 fuerza entrada a estado Fault para demo/prueba. */
        if (dip_fault_forced(shared_data)) {
            fault_elapsed_ms = 0u;
            fault_blink_ms = 0u;
            fault_led_on = true;
            shared_data->alarm_on = true;
            TEST_LOG_SYS("[SYS] DIP4=1 -> force ST_FAULT\r\n");
            state = ST_FAULT;
            break;
        }

        if (shared_data->ev_light_on_pressed) {
            shared_data->ev_light_on_pressed = false;
            shared_data->ev_sys_pressed = false;
            shared_data->light_enabled = true;
            shared_data->ev_send_bt_update_light = true;
            shared_data->flash_save_light_request = true;
            TEST_LOG_SYS("[SYS] light ON t=%lu ms\r\n", HAL_GetTick());
        }

        if (shared_data->ev_light_off_pressed) {
            shared_data->ev_light_off_pressed = false;
            shared_data->ev_sys_pressed = false;
            shared_data->light_enabled = false;
            shared_data->ev_send_bt_update_light = true;
            shared_data->flash_save_light_request = true;
            TEST_LOG_SYS("[SYS] light OFF t=%lu ms\r\n", HAL_GetTick());
        }

        if (shared_data->ev_send_bt_update_light) {
            TEST_LOG_SYS("[SYS] light state=%u\r\n",
                         shared_data->light_enabled ? 1u : 0u);
        }

        if (shared_data->ev_pote_changed) {
            shared_data->ev_pote_changed = false;
            shared_data->ev_send_bt_update_pote = true;
            TEST_LOG_SYS("[SYS] pote changed adc=%u%% delay=%u us\r\n",
                         shared_data->adc_percent,
                         shared_data->fan_delay_us);
        }

        if (shared_data->flash_save_light_request || shared_data->flash_save_adc_calib_request) {
            bool light_save_req = shared_data->flash_save_light_request;
            bool adc_calib_save_req = shared_data->flash_save_adc_calib_request;

            shared_data->flash_save_light_request = false;
            shared_data->flash_save_adc_calib_request = false;

            if (!flash_store_persistent_state(shared_data->light_enabled,
                                              shared_data->adc_calib_valid,
                                              shared_data->adc_calib_min,
                                              shared_data->adc_calib_max)) {
#if APP_FLASH_STORE_STRICT
                /* En modo estricto, la persistencia de luz fallida es crítica. */
                if (light_save_req) {
                    fault_elapsed_ms = 0u;
                    fault_blink_ms = 0u;
                    fault_led_on = true;
                    shared_data->alarm_on = true;
                    state = ST_FAULT;
                }
#endif
                TEST_LOG_SYS("[SYS] flash save FAILED (light_req=%u calib_req=%u)\r\n",
                             light_save_req ? 1u : 0u,
                             adc_calib_save_req ? 1u : 0u);
            } else {
                TEST_LOG_SYS("[SYS] flash save OK light=%u calib=%u min=%u max=%u\r\n",
                             shared_data->light_enabled ? 1u : 0u,
                             shared_data->adc_calib_valid ? 1u : 0u,
                             shared_data->adc_calib_min,
                             shared_data->adc_calib_max);
            }
        }
        break;

    case ST_FAULT:
        /* Modo seguro: corta potencia y parpadea alarma 1 Hz. */
        apply_dip_roles(shared_data);

        /* Si DIP4 vuelve a 0, sale de Fault inmediatamente. */
        if (!dip_fault_forced(shared_data)) {
            shared_data->fault_mode = false;
            shared_data->cut_off_voltage = false;
            shared_data->alarm_on = false;
            fault_elapsed_ms = 0u;
            fault_blink_ms = 0u;
            fault_led_on = false;
            TEST_LOG_SYS("[SYS] DIP4=0 -> leave ST_FAULT\r\n");
            state = ST_NORMAL;
            break;
        }

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
static bool flash_load_persistent_state(shared_data_type *shared_data)
{
    uint32_t magic = *(volatile uint32_t *)FLASH_STATE_MAGIC_ADDR;
    uint32_t meta = *(volatile uint32_t *)FLASH_STATE_META_ADDR;

    shared_data->light_enabled = false;
    shared_data->adc_calib_valid = false;
    shared_data->adc_calib_min = 0u;
    shared_data->adc_calib_max = ADC_FULL_SCALE_COUNTS;

    if (magic != FLASH_MAGIC_WORD) {
        return true;
    }

    if (meta == FLASH_LAYOUT_VERSION) {
        uint32_t light_word = *(volatile uint32_t *)FLASH_STATE_LIGHT_ADDR;
        uint32_t adc_min_word = *(volatile uint32_t *)FLASH_STATE_ADC_MIN_ADDR;
        uint32_t adc_max_word = *(volatile uint32_t *)FLASH_STATE_ADC_MAX_ADDR;

        shared_data->light_enabled = ((light_word & 0x1u) != 0u);

        if ((adc_min_word != FLASH_ADC_INVALID_WORD) &&
            (adc_max_word != FLASH_ADC_INVALID_WORD) &&
            (adc_min_word <= ADC_FULL_SCALE_COUNTS) &&
            (adc_max_word <= ADC_FULL_SCALE_COUNTS) &&
            (adc_max_word > adc_min_word)) {
            shared_data->adc_calib_valid = true;
            shared_data->adc_calib_min = (uint16_t)adc_min_word;
            shared_data->adc_calib_max = (uint16_t)adc_max_word;
        }
    } else {
        /* Compatibilidad retro: layout viejo (magic + light). */
        shared_data->light_enabled = ((meta & 0x1u) != 0u);
    }

    return true;
}

static bool flash_store_persistent_state(bool light_on,
                                         bool adc_calib_valid,
                                         uint16_t adc_calib_min,
                                         uint16_t adc_calib_max)
{
    HAL_StatusTypeDef hal_status;
    FLASH_EraseInitTypeDef erase_cfg;
    uint32_t page_error = 0u;
    uint32_t adc_min_word = FLASH_ADC_INVALID_WORD;
    uint32_t adc_max_word = FLASH_ADC_INVALID_WORD;

    if (adc_calib_valid &&
        (adc_calib_max > adc_calib_min) &&
        (adc_calib_max <= ADC_FULL_SCALE_COUNTS)) {
        adc_min_word = adc_calib_min;
        adc_max_word = adc_calib_max;
    }

    /* Borra y reescribe una página dedicada con layout versionado. */
    hal_status = HAL_FLASH_Unlock();
    if (hal_status != HAL_OK) {
        return false;
    }

    erase_cfg.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_cfg.PageAddress = FLASH_STATE_PAGE_ADDR;
    erase_cfg.NbPages = 1u;
    hal_status = HAL_FLASHEx_Erase(&erase_cfg, &page_error);
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STATE_MAGIC_ADDR, FLASH_MAGIC_WORD);
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STATE_META_ADDR, FLASH_LAYOUT_VERSION);
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STATE_LIGHT_ADDR, (uint32_t)(light_on ? 1u : 0u));
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STATE_ADC_MIN_ADDR, adc_min_word);
    if (hal_status != HAL_OK) {
        (void)HAL_FLASH_Lock();
        return false;
    }

    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STATE_ADC_MAX_ADDR, adc_max_word);
    (void)HAL_FLASH_Lock();

    return (hal_status == HAL_OK);
}

static uint16_t build_fan_delay_from_percent(uint8_t adc_percent)
{
    uint8_t bounded_percent = (adc_percent > 100u) ? 100u : adc_percent;

    return (uint16_t)(APP_FAN_DIM_DELAY_MIN_US +
                      (((uint32_t)bounded_percent * (APP_FAN_DIM_DELAY_MAX_US - APP_FAN_DIM_DELAY_MIN_US)) / 100u));
}

static void apply_dip_roles(shared_data_type *shared_data)
{
    /* DIP role map:
     * DIP1 -> Bluetooth enable
     * DIP2 -> Buzzer enable
     * DIP3 -> LED enable
     * DIP4 -> Force FAULT mode (handled by FSM)
     */
    shared_data->bt_enabled = ((shared_data->dip_value & (1u << 0)) != 0u);
    shared_data->buzzer_enabled = ((shared_data->dip_value & (1u << 1)) != 0u);
    shared_data->led_enabled = ((shared_data->dip_value & (1u << 2)) != 0u);
}

static bool dip_fault_forced(const shared_data_type *shared_data)
{
    return ((shared_data->dip_value & (1u << 3)) != 0u);
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
