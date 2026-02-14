/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 */

/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include <stdio.h>
#include <string.h>

/* Application & Tasks includes. */
#include "app.h"

#if APP_TEST_MODE
#define TEST_LOG(...) LOGGER_LOG(__VA_ARGS__)
#else
#define TEST_LOG(...)
#endif

/********************** macros and definitions *******************************/
#define UART_TX_TIMEOUT_MS  20u
#define BUZZER_FREQ_HZ      2000u
#define BUZZER_DUTY_PERCENT 30u
#define ZC_PERIOD_MIN_US    7000u
#define ZC_PERIOD_MAX_US    13000u

/* Role mapping: swap these two pairs if board wiring is opposite. */
#define TRIAC_LIGHT_PIN      TRIAC2_Pin
#define TRIAC_LIGHT_PORT     TRIAC2_GPIO_Port
#define TRIAC_FAN_PIN        TRIAC1_Pin
#define TRIAC_FAN_PORT       TRIAC1_GPIO_Port

typedef struct {
    const char *name;
    uint16_t pin;
    GPIO_TypeDef *port;
    volatile bool on_pending;
    volatile bool off_pending;
} triac_channel_t;

/********************** internal functions declaration ***********************/
static uint32_t clamp_u32(uint32_t value, uint32_t min, uint32_t max);
static void tim2_config_tick_1mhz(void);
static void triac_outputs_off(void);
static void triac_disable_all_events(void);
static void triac_cancel_light_events(void);
static void triac_cancel_fan_events(void);
static void bt_send_status(const shared_data_type *shared_data);
static void buzzer_start_tone(void);
static void buzzer_stop_tone(void);

/********************** internal data definition *****************************/
const char *p_task_pwm = "Task Triac";

static triac_channel_t triac_fan = {"FAN", TRIAC_FAN_PIN, TRIAC_FAN_PORT, false, false};
static triac_channel_t triac_light = {"LIGHT", TRIAC_LIGHT_PIN, TRIAC_LIGHT_PORT, false, false};

/********************** external data declaration *****************************/
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

/********************** external functions definition ************************/
void task_pwm_init(void *parameters)
{
    (void)parameters;

    TEST_LOG("  %s is running - %s\r\n", GET_NAME(task_pwm_init), p_task_pwm);

    triac_outputs_off();
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    tim2_config_tick_1mhz();
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    __HAL_TIM_SET_AUTORELOAD(&htim2, APP_ZC_HALF_CYCLE_US - 1u);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, APP_ZC_HALF_CYCLE_US - 1u);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, APP_ZC_HALF_CYCLE_US - 1u);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, APP_ZC_HALF_CYCLE_US - 1u);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, APP_ZC_HALF_CYCLE_US - 1u);
    (void)HAL_TIM_Base_Start(&htim2);

    triac_disable_all_events();
}

void task_pwm_update(void *parameters)
{
    static bool buzzer_running = false;
    shared_data_type *shared_data = (shared_data_type *)parameters;
    bool zc_pending = false;
    uint32_t zc_timestamp_us = 0u;

    /* Consume flag de ZC solo para trazas de depuración del scheduler. */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (shared_data->zc_event_pending) {
        zc_pending = true;
        zc_timestamp_us = shared_data->zc_timestamp_us;
        shared_data->zc_event_pending = false;
    }
    if (primask == 0u) {
        __enable_irq();
    }

    if (zc_pending) {
        TEST_LOG("[PWM] ZC captured t=%lu us\r\n", zc_timestamp_us);
    }

    if (shared_data->cut_off_voltage || shared_data->fault_mode) {
        /* En Fault/seguridad se bloquea toda salida de potencia. */
        triac_disable_all_events();
        triac_outputs_off();

        HAL_GPIO_WritePin(LED_GPIO_Port,
                          LED_Pin,
                          (shared_data->led_enabled && shared_data->alarm_on) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        if (shared_data->alarm_on && shared_data->buzzer_enabled) {
            /* Buzzer pasivo: requiere PWM (tono) para sonar. */
            if (!buzzer_running) {
                buzzer_start_tone();
                buzzer_running = true;
            }
        } else if (buzzer_running) {
            buzzer_stop_tone();
            buzzer_running = false;
        }
    } else {
        /* Cancela eventos pendientes si se apagó una salida a mitad de semiciclo. */
        if (!shared_data->light_enabled) {
            triac_cancel_light_events();
        }
        if (!shared_data->fan_enabled) {
            triac_cancel_fan_events();
        }

        if (buzzer_running) {
            buzzer_stop_tone();
            buzzer_running = false;
        }
        HAL_GPIO_WritePin(LED_GPIO_Port,
                          LED_Pin,
                          (shared_data->led_enabled && shared_data->light_enabled) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    if (shared_data->bt_enabled &&
        (shared_data->ev_send_bt_update_light || shared_data->ev_send_bt_update_pote)) {
        /* Telemetría BT solo cuando hay cambios relevantes. */
        bt_send_status(shared_data);
        shared_data->ev_send_bt_update_light = false;
        shared_data->ev_send_bt_update_pote = false;
    }
}

void task_pwm_on_zero_crossing_isr(const shared_data_type *shared_data)
{
    uint32_t zc_period_us;
    uint32_t triac_fire_max_us;
    uint32_t fan_delay_max_us;
    uint32_t fan_delay_us;
    uint32_t light_on_us;
    uint32_t light_off_us;
    uint32_t fan_on_us;
    uint32_t fan_off_us;
    uint32_t enabled_events = 0u;

    if (shared_data == NULL) {
        return;
    }

    if (shared_data->fault_mode || shared_data->cut_off_voltage) {
        triac_disable_all_events();
        triac_outputs_off();
        return;
    }

    zc_period_us = clamp_u32(shared_data->zc_period_us, ZC_PERIOD_MIN_US, ZC_PERIOD_MAX_US);
    triac_fire_max_us = zc_period_us - APP_TRIAC_PULSE_US - 1u;
    fan_delay_max_us = (triac_fire_max_us > APP_TRIAC_FIXED_WAIT_US) ?
                       (triac_fire_max_us - APP_TRIAC_FIXED_WAIT_US) : 0u;

    fan_delay_us = clamp_u32(shared_data->fan_delay_us, APP_FAN_DIM_DELAY_MIN_US, APP_FAN_DIM_DELAY_MAX_US);
    fan_delay_us = clamp_u32(fan_delay_us, APP_FAN_DIM_DELAY_MIN_US, fan_delay_max_us);

    light_on_us = clamp_u32(APP_TRIAC_FIXED_WAIT_US, 0u, triac_fire_max_us);
    fan_on_us = clamp_u32(APP_TRIAC_FIXED_WAIT_US + fan_delay_us, 0u, triac_fire_max_us);
    light_off_us = light_on_us + APP_TRIAC_PULSE_US;
    fan_off_us = fan_on_us + APP_TRIAC_PULSE_US;

    triac_disable_all_events();
    triac_outputs_off();

    __HAL_TIM_SET_AUTORELOAD(&htim2, zc_period_us - 1u);
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, light_on_us);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, light_off_us);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, fan_on_us);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, fan_off_us);

    triac_light.on_pending = shared_data->light_enabled;
    triac_light.off_pending = shared_data->light_enabled;
    triac_fan.on_pending = shared_data->fan_enabled;
    triac_fan.off_pending = shared_data->fan_enabled;

    if (triac_light.on_pending) {
        enabled_events |= TIM_IT_CC1;
    }
    if (triac_light.off_pending) {
        enabled_events |= TIM_IT_CC2;
    }
    if (triac_fan.on_pending) {
        enabled_events |= TIM_IT_CC3;
    }
    if (triac_fan.off_pending) {
        enabled_events |= TIM_IT_CC4;
    }

    if (enabled_events != 0u) {
        __HAL_TIM_ENABLE_IT(&htim2, enabled_events);
    }
}

void task_pwm_timer_isr(void)
{
    uint32_t sr = htim2.Instance->SR;
    uint32_t dier = htim2.Instance->DIER;

    if ((sr & TIM_SR_CC1IF) && (dier & TIM_DIER_CC1IE)) {
        htim2.Instance->SR &= ~TIM_SR_CC1IF;
        if (triac_light.on_pending) {
            HAL_GPIO_WritePin(triac_light.port, triac_light.pin, GPIO_PIN_SET);
            triac_light.on_pending = false;
            TEST_LOG("[PWM] %s FIRE ON t=%lu us\r\n", triac_light.name, __HAL_TIM_GET_COUNTER(&htim2));
        }
        htim2.Instance->DIER &= ~TIM_DIER_CC1IE;
    }

    if ((sr & TIM_SR_CC2IF) && (dier & TIM_DIER_CC2IE)) {
        htim2.Instance->SR &= ~TIM_SR_CC2IF;
        HAL_GPIO_WritePin(triac_light.port, triac_light.pin, GPIO_PIN_RESET);
        triac_light.off_pending = false;
        TEST_LOG("[PWM] %s FIRE OFF t=%lu us\r\n", triac_light.name, __HAL_TIM_GET_COUNTER(&htim2));
        htim2.Instance->DIER &= ~TIM_DIER_CC2IE;
    }

    if ((sr & TIM_SR_CC3IF) && (dier & TIM_DIER_CC3IE)) {
        htim2.Instance->SR &= ~TIM_SR_CC3IF;
        if (triac_fan.on_pending) {
            HAL_GPIO_WritePin(triac_fan.port, triac_fan.pin, GPIO_PIN_SET);
            triac_fan.on_pending = false;
            TEST_LOG("[PWM] %s FIRE ON t=%lu us\r\n", triac_fan.name, __HAL_TIM_GET_COUNTER(&htim2));
        }
        htim2.Instance->DIER &= ~TIM_DIER_CC3IE;
    }

    if ((sr & TIM_SR_CC4IF) && (dier & TIM_DIER_CC4IE)) {
        htim2.Instance->SR &= ~TIM_SR_CC4IF;
        HAL_GPIO_WritePin(triac_fan.port, triac_fan.pin, GPIO_PIN_RESET);
        triac_fan.off_pending = false;
        TEST_LOG("[PWM] %s FIRE OFF t=%lu us\r\n", triac_fan.name, __HAL_TIM_GET_COUNTER(&htim2));
        htim2.Instance->DIER &= ~TIM_DIER_CC4IE;
    }
}

/********************** internal functions definition ************************/
static uint32_t clamp_u32(uint32_t value, uint32_t min, uint32_t max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static void tim2_config_tick_1mhz(void)
{
    uint32_t pclk1_hz = HAL_RCC_GetPCLK1Freq();
    uint32_t tim_clk_hz = pclk1_hz;
    uint32_t prescaler;

    /* En STM32F1, si APB1 no es /1, el clock efectivo de TIM2 se duplica. */
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) {
        tim_clk_hz *= 2u;
    }

    prescaler = tim_clk_hz / 1000000u;
    if (prescaler == 0u) {
        prescaler = 1u;
    }

    __HAL_TIM_DISABLE(&htim2);
    __HAL_TIM_SET_PRESCALER(&htim2, prescaler - 1u);
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    htim2.Instance->EGR = TIM_EGR_UG;
}

static void triac_outputs_off(void)
{
    HAL_GPIO_WritePin(triac_light.port, triac_light.pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(triac_fan.port, triac_fan.pin, GPIO_PIN_RESET);
}

static void triac_disable_all_events(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    triac_light.on_pending = false;
    triac_light.off_pending = false;
    triac_fan.on_pending = false;
    triac_fan.off_pending = false;

    htim2.Instance->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE);
    htim2.Instance->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF | TIM_SR_CC4IF);

    if (primask == 0u) {
        __enable_irq();
    }
}

static void triac_cancel_light_events(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    triac_light.on_pending = false;
    triac_light.off_pending = false;
    htim2.Instance->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE);
    htim2.Instance->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF);
    if (primask == 0u) {
        __enable_irq();
    }

    HAL_GPIO_WritePin(triac_light.port, triac_light.pin, GPIO_PIN_RESET);
}

static void triac_cancel_fan_events(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    triac_fan.on_pending = false;
    triac_fan.off_pending = false;
    htim2.Instance->DIER &= ~(TIM_DIER_CC3IE | TIM_DIER_CC4IE);
    htim2.Instance->SR &= ~(TIM_SR_CC3IF | TIM_SR_CC4IF);
    if (primask == 0u) {
        __enable_irq();
    }

    HAL_GPIO_WritePin(triac_fan.port, triac_fan.pin, GPIO_PIN_RESET);
}

static void bt_send_status(const shared_data_type *shared_data)
{
    char tx[80];
    int len = snprintf(tx,
                       sizeof(tx),
                       /* Formato simple para app móvil: JSON por línea. */
                       "{\"adc\":%u,\"light\":%u,\"btCfg\":%u}\r\n",
                       (unsigned int)shared_data->adc_percent,
                       (unsigned int)(shared_data->light_enabled ? 1u : 0u),
                       (unsigned int)((shared_data->dip_value >> 3u) & 0x01u));

    if (len > 0) {
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)tx, (uint16_t)strlen(tx), UART_TX_TIMEOUT_MS);
        TEST_LOG("[BT] TX %s", tx);
    }
}

static void buzzer_start_tone(void)
{
    uint32_t pclk2_hz = HAL_RCC_GetPCLK2Freq();
    uint32_t timer_clock_hz = pclk2_hz;
    uint32_t timer_tick_hz;
    uint32_t period_ticks;
    uint32_t pulse_ticks;
    uint32_t prescaler = (uint32_t)htim1.Instance->PSC + 1u;

    /* En STM32F1, si APB2 no es /1, el clock efectivo de timer se duplica. */
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1) {
        timer_clock_hz *= 2u;
    }

    timer_tick_hz = timer_clock_hz / prescaler;
    period_ticks = timer_tick_hz / BUZZER_FREQ_HZ;
    if (period_ticks < 2u) {
        period_ticks = 2u;
    }
    pulse_ticks = (period_ticks * BUZZER_DUTY_PERCENT) / 100u;
    if (pulse_ticks < 1u) {
        pulse_ticks = 1u;
    }

    __HAL_TIM_SET_AUTORELOAD(&htim1, period_ticks - 1u);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_ticks);
    __HAL_TIM_SET_COUNTER(&htim1, 0u);
    (void)HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void buzzer_stop_tone(void)
{
    (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0u);
}

/********************** end of file ******************************************/
