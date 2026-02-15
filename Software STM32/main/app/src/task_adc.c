/* Project includes. */
#include "main.h"

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
#define ADC_PERIOD_MS            50u
#define BTN_DEBOUNCE_TICKS       50u
#define BUTTON_PRESSED_LEVEL     GPIO_PIN_RESET
#define ADC_FULL_SCALE_COUNTS    4095u
/* Ajuste manual del rango útil del potenciómetro (cuentas ADC crudas: 0..4095). */
#define ADC_RAW_MIN_COUNTS       696u
#define ADC_RAW_MAX_COUNTS       3194u

typedef enum {
    ST_BTN_UNPRESSED = 0,
    ST_BTN_FALLING,
    ST_BTN_PRESSED,
    ST_BTN_RISING
} button_state_t;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    button_state_t state;
    uint8_t tick;
} button_ctx_t;

/********************** internal functions declaration ***********************/
static HAL_StatusTypeDef adc_poll_read(uint16_t *value);
static bool update_button_fsm(button_ctx_t *ctx);
static uint8_t adc_to_percent(uint16_t raw);

/********************** internal data definition *****************************/
const char *p_task_adc = "Task Sensor";

/********************** external data declaration *****************************/
extern ADC_HandleTypeDef hadc1;

/********************** external functions definition ************************/
void task_adc_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *)parameters;

    TEST_LOG("  %s is running - %s\r\n", GET_NAME(task_adc_init), p_task_adc);

    shared_data->adc_raw = 0u;
    shared_data->adc_percent = 0u;
    shared_data->fan_delay_us = 0u;
    shared_data->ev_sys_pressed = false;
    shared_data->ev_light_on_pressed = false;
    shared_data->ev_light_off_pressed = false;
    shared_data->ev_pote_changed = false;
}

void task_adc_update(void *parameters)
{
    static uint32_t adc_tick = 0u;
    static uint8_t last_percent = 0u;
    static uint32_t last_sensor_log_ms = 0u;
    static button_ctx_t btn_on  = {BOT1_GPIO_Port, BOT1_Pin, ST_BTN_UNPRESSED, 0u};
    static button_ctx_t btn_off = {BUT2_GPIO_Port, BUT2_Pin, ST_BTN_UNPRESSED, 0u};

    shared_data_type *shared_data = (shared_data_type *)parameters;
    uint16_t adc_value = 0u;
    uint8_t adc_percent = 0u;

    /* Lee el estado instantáneo de los 4 DIP switches. */
    shared_data->dip_value =
        ((HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin) == GPIO_PIN_SET) ? (1u << 0) : 0u) |
        ((HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin) == GPIO_PIN_SET) ? (1u << 1) : 0u) |
        ((HAL_GPIO_ReadPin(DIP3_GPIO_Port, DIP3_Pin) == GPIO_PIN_SET) ? (1u << 2) : 0u) |
        ((HAL_GPIO_ReadPin(DIP4_GPIO_Port, DIP4_Pin) == GPIO_PIN_SET) ? (1u << 3) : 0u);

    /* Debounce por máquina de estados para botón ON (BOT1) y OFF (BUT2). */
    if (update_button_fsm(&btn_on)) {
        shared_data->ev_light_on_pressed = true;
        shared_data->ev_sys_pressed = true; /* compatibilidad con logs existentes */
        TEST_LOG("[ADC] EV_LIGHT_ON_PRESSED t=%lu ms\r\n", HAL_GetTick());
    }
    if (update_button_fsm(&btn_off)) {
        shared_data->ev_light_off_pressed = true;
        shared_data->ev_sys_pressed = true; /* compatibilidad con logs existentes */
        TEST_LOG("[ADC] EV_LIGHT_OFF_PRESSED t=%lu ms\r\n", HAL_GetTick());
    }

    /* Muestreo periódico del potenciómetro (ADC). */
    adc_tick++;
    if (adc_tick >= ADC_PERIOD_MS) {
        adc_tick = 0u;

        if (HAL_OK == adc_poll_read(&adc_value)) {
            shared_data->adc_raw = adc_value;
            adc_percent = adc_to_percent(adc_value);
            shared_data->adc_percent = adc_percent;
            shared_data->fan_delay_us = (uint16_t)(APP_FAN_DIM_DELAY_MIN_US +
                                          (((uint32_t)adc_percent * (APP_FAN_DIM_DELAY_MAX_US - APP_FAN_DIM_DELAY_MIN_US)) / 100u));

            /* Solo levanta evento si hubo cambio real en el valor mapeado. */
            if (adc_percent != last_percent) {
                last_percent = adc_percent;
                shared_data->ev_pote_changed = true;
                TEST_LOG("[ADC] EV_POTE_CHANGED adc=%u%% delay=%u us t=%lu ms\r\n",
                         shared_data->adc_percent,
                         shared_data->fan_delay_us,
                         HAL_GetTick());
            }
        }
    }

    if ((HAL_GetTick() - last_sensor_log_ms) >= 1000u) {
        last_sensor_log_ms = HAL_GetTick();
        TEST_LOG("[ADC] heartbeat dip=0x%X adc=%u%% min=%u max=%u on_ev=%u off_ev=%u\r\n",
                 shared_data->dip_value,
                 shared_data->adc_percent,
                 shared_data->adc_calib_min,
                 shared_data->adc_calib_max,
                 shared_data->ev_light_on_pressed ? 1u : 0u,
                 shared_data->ev_light_off_pressed ? 1u : 0u);
    }
}

/********************** internal functions definition ************************/
static HAL_StatusTypeDef adc_poll_read(uint16_t *value)
{
    HAL_StatusTypeDef res = HAL_ADC_Start(&hadc1);

    if (HAL_OK == res) {
        res = HAL_ADC_PollForConversion(&hadc1, 0u);
        if (HAL_OK == res) {
            *value = (uint16_t)HAL_ADC_GetValue(&hadc1);
        }
        (void)HAL_ADC_Stop(&hadc1);
    }

    return res;
}

static bool update_button_fsm(button_ctx_t *ctx)
{
    bool btn_pressed = (HAL_GPIO_ReadPin(ctx->port, ctx->pin) == BUTTON_PRESSED_LEVEL);

    /* FSM de debounce estilo Harel: UNPRESSED/FALLING/PRESSED/RISING. */
    switch (ctx->state) {
    case ST_BTN_UNPRESSED:
        if (btn_pressed) {
            ctx->tick = BTN_DEBOUNCE_TICKS;
            ctx->state = ST_BTN_FALLING;
        }
        break;

    case ST_BTN_FALLING:
        if (btn_pressed) {
            if (ctx->tick > 0u) {
                ctx->tick--;
            } else {
                ctx->state = ST_BTN_PRESSED;
                return true;
            }
        } else {
            ctx->state = ST_BTN_UNPRESSED;
        }
        break;

    case ST_BTN_PRESSED:
        if (!btn_pressed) {
            ctx->tick = BTN_DEBOUNCE_TICKS;
            ctx->state = ST_BTN_RISING;
        }
        break;

    case ST_BTN_RISING:
        if (!btn_pressed) {
            if (ctx->tick > 0u) {
                ctx->tick--;
            } else {
                ctx->state = ST_BTN_UNPRESSED;
            }
        } else {
            ctx->state = ST_BTN_PRESSED;
        }
        break;

    default:
        ctx->state = ST_BTN_UNPRESSED;
        break;
    }

    return false;
}

static uint8_t adc_to_percent(uint16_t raw)
{
    uint32_t min_counts = ADC_RAW_MIN_COUNTS;
    uint32_t max_counts = ADC_RAW_MAX_COUNTS;
    uint32_t bounded_raw = raw;

    if (max_counts <= min_counts) {
        return 0u;
    }

    if (bounded_raw < min_counts) {
        bounded_raw = min_counts;
    } else if (bounded_raw > max_counts) {
        bounded_raw = max_counts;
    }

    return (uint8_t)(((bounded_raw - min_counts) * 100u) / (max_counts - min_counts));
}

/********************** end of file ******************************************/
