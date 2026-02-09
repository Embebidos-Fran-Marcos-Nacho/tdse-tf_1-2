/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"

/* Application & Tasks includes. */
#include "app.h"

/********************** macros and definitions *******************************/
#define ADC_PERIOD_MS           50u
#define BTN_DEBOUNCE_TICKS      50u
#define BUTTON_PRESSED_LEVEL    GPIO_PIN_RESET

typedef enum {
    ST_BTN_UNPRESSED = 0,
    ST_BTN_FALLING,
    ST_BTN_PRESSED,
    ST_BTN_RISING
} button_state_t;

/********************** internal functions declaration ***********************/
static HAL_StatusTypeDef adc_poll_read(uint16_t *value);
static void update_button_fsm(shared_data_type *shared_data);

/********************** internal data definition *****************************/
const char *p_task_adc = "Task Sensor";

/********************** external data declaration *****************************/
extern ADC_HandleTypeDef hadc1;

/********************** external functions definition ************************/
void task_adc_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *)parameters;

    LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_adc_init), p_task_adc);

    shared_data->adc_raw = 0u;
    shared_data->adc_percent = 0u;
    shared_data->fan_delay_us = 0u;
    shared_data->ev_sys_pressed = false;
    shared_data->ev_pote_changed = false;
}

void task_adc_update(void *parameters)
{
    static uint32_t adc_tick = 0u;
    static uint8_t last_percent = 0u;

    shared_data_type *shared_data = (shared_data_type *)parameters;
    uint16_t adc_value = 0u;
    uint8_t adc_percent = 0u;

    /* Lee el estado instantáneo de los 4 DIP switches. */
    shared_data->dip_value =
        ((HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin) == GPIO_PIN_SET) ? (1u << 0) : 0u) |
        ((HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin) == GPIO_PIN_SET) ? (1u << 1) : 0u) |
        ((HAL_GPIO_ReadPin(DIP3_GPIO_Port, DIP3_Pin) == GPIO_PIN_SET) ? (1u << 2) : 0u) |
        ((HAL_GPIO_ReadPin(DIP4_GPIO_Port, DIP4_Pin) == GPIO_PIN_SET) ? (1u << 3) : 0u);

    /* Debounce por máquina de estados del botón de luz. */
    update_button_fsm(shared_data);

    /* Muestreo periódico del potenciómetro (ADC). */
    adc_tick++;
    if (adc_tick >= ADC_PERIOD_MS) {
        adc_tick = 0u;

        if (HAL_OK == adc_poll_read(&adc_value)) {
            shared_data->adc_raw = adc_value;
            shared_data->fan_delay_us = (uint16_t)((adc_value * 7500u) / 4095u);

            adc_percent = (uint8_t)((adc_value * 100u) / 4095u);
            shared_data->adc_percent = adc_percent;

            /* Solo levanta evento si hubo cambio real en el valor mapeado. */
            if (adc_percent != last_percent) {
                last_percent = adc_percent;
                shared_data->ev_pote_changed = true;
            }
        }
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

static void update_button_fsm(shared_data_type *shared_data)
{
    static button_state_t state = ST_BTN_UNPRESSED;
    static uint8_t tick = 0u;

    bool btn_pressed = (HAL_GPIO_ReadPin(BOT1_GPIO_Port, BOT1_Pin) == BUTTON_PRESSED_LEVEL);

    /* FSM de debounce estilo Harel: UNPRESSED/FALLING/PRESSED/RISING. */
    switch (state) {
    case ST_BTN_UNPRESSED:
        if (btn_pressed) {
            tick = BTN_DEBOUNCE_TICKS;
            state = ST_BTN_FALLING;
        }
        break;

    case ST_BTN_FALLING:
        if (btn_pressed) {
            if (tick > 0u) {
                tick--;
            } else {
                /* Evento limpio de pulsación confirmada. */
                shared_data->ev_sys_pressed = true;
                state = ST_BTN_PRESSED;
            }
        } else {
            state = ST_BTN_UNPRESSED;
        }
        break;

    case ST_BTN_PRESSED:
        if (!btn_pressed) {
            tick = BTN_DEBOUNCE_TICKS;
            state = ST_BTN_RISING;
        }
        break;

    case ST_BTN_RISING:
        if (!btn_pressed) {
            if (tick > 0u) {
                tick--;
            } else {
                state = ST_BTN_UNPRESSED;
            }
        } else {
            state = ST_BTN_PRESSED;
        }
        break;

    default:
        state = ST_BTN_UNPRESSED;
        break;
    }
}

/********************** end of file ******************************************/
