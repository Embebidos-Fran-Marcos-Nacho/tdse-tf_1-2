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
#define TRIAC_FIXED_WAIT_US 0ul
#define TRIAC_DELAY_MIN_US  0ul
#define TRIAC_DELAY_MAX_US  8000ul
#define TRIAC_PULSE_US      400ul
#define WAIT_SPIN_US        200ul
#define UART_TX_TIMEOUT_MS  20u
#define BUZZER_FREQ_HZ      2000u
#define BUZZER_DUTY_PERCENT 30u

/* Role mapping: swap these two pairs if board wiring is opposite. */
#define TRIAC_LIGHT_PIN      TRIAC2_Pin
#define TRIAC_LIGHT_PORT     TRIAC2_GPIO_Port
#define TRIAC_FAN_PIN        TRIAC1_Pin
#define TRIAC_FAN_PORT       TRIAC1_GPIO_Port

typedef struct {
    const char *name;
    uint16_t pin;
    GPIO_TypeDef *port;
    bool armed;
    bool pulsing;
    uint32_t fire_at_us;
    uint32_t pulse_off_us;
} triac_channel_t;

/********************** internal functions declaration ***********************/
static bool time_reached(uint32_t now_us, uint32_t target_us);
static void triac_schedule(triac_channel_t *channel, uint32_t zc_time_us, uint32_t delay_us);
static void triac_update_channel(triac_channel_t *channel, bool enabled);
static void bt_send_status(const shared_data_type *shared_data);
static void buzzer_start_tone(void);
static void buzzer_stop_tone(void);

/********************** internal data definition *****************************/
const char *p_task_pwm = "Task Triac";

/********************** external data declaration *****************************/
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;

/********************** external functions definition ************************/
void task_pwm_init(void *parameters)
{
    (void)parameters;

    TEST_LOG("  %s is running - %s\r\n", GET_NAME(task_pwm_init), p_task_pwm);

    HAL_GPIO_WritePin(TRIAC1_GPIO_Port, TRIAC1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TRIAC2_GPIO_Port, TRIAC2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void task_pwm_update(void *parameters)
{
    static triac_channel_t triac_fan = {"FAN", TRIAC_FAN_PIN, TRIAC_FAN_PORT, false, false, 0u, 0u};
    static triac_channel_t triac_light = {"LIGHT", TRIAC_LIGHT_PIN, TRIAC_LIGHT_PORT, false, false, 0u, 0u};
    static bool buzzer_running = false;

    shared_data_type *shared_data = (shared_data_type *)parameters;
    bool zc_pending = false;
    uint32_t zc_timestamp_us = 0u;

    /* Toma el evento de cruce por cero de forma atómica. */
    __disable_irq();
    if (shared_data->zc_event_pending) {
        zc_pending = true;
        zc_timestamp_us = shared_data->zc_timestamp_us;
        shared_data->zc_event_pending = false;
    }
    __enable_irq();

    if (zc_pending) {
        TEST_LOG("[PWM] ZC event t_zc=%lu us, t_now=%lu us\r\n",
                 zc_timestamp_us,
                 app_get_time_us());

        if (shared_data->fan_enabled) {
            /* Fan: disparo con dimming (offset fijo + delay del pote). */
            triac_schedule(&triac_fan, zc_timestamp_us, shared_data->fan_delay_us);
        } else {
            triac_fan.armed = false;
            triac_fan.pulsing = false;
            HAL_GPIO_WritePin(triac_fan.port, triac_fan.pin, GPIO_PIN_RESET);
        }

        if (shared_data->light_enabled) {
            /* Luz: disparo no dimerizable (solo offset fijo post-ZC). */
            triac_schedule(&triac_light, zc_timestamp_us, 0u);
        } else {
            triac_light.armed = false;
            triac_light.pulsing = false;
            HAL_GPIO_WritePin(triac_light.port, triac_light.pin, GPIO_PIN_RESET);
        }
    }

    if (shared_data->cut_off_voltage || shared_data->fault_mode) {
        /* En Fault/seguridad se bloquea toda salida de potencia. */
        triac_fan.armed = false;
        triac_fan.pulsing = false;
        triac_light.armed = false;
        triac_light.pulsing = false;

        HAL_GPIO_WritePin(triac_fan.port, triac_fan.pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(triac_light.port, triac_light.pin, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(LED_GPIO_Port,
                          LED_Pin,
                          (shared_data->led_enabled && shared_data->alarm_on) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        if (shared_data->alarm_on && shared_data->buzzer_enabled) {
            /* Buzzer pasivo: requiere PWM (tono) para sonar. */
            if (!buzzer_running) {
                buzzer_start_tone();
                buzzer_running = true;
            }
        } else {
            if (buzzer_running) {
                buzzer_stop_tone();
                buzzer_running = false;
            }
        }
    } else {
        triac_update_channel(&triac_fan, shared_data->fan_enabled);
        triac_update_channel(&triac_light, shared_data->light_enabled);

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

/********************** internal functions definition ************************/
static bool time_reached(uint32_t now_us, uint32_t target_us)
{
    return ((int32_t)(now_us - target_us) >= 0);
}

static void triac_schedule(triac_channel_t *channel, uint32_t zc_time_us, uint32_t delay_us)
{
    if (delay_us < TRIAC_DELAY_MIN_US) {
        delay_us = TRIAC_DELAY_MIN_US;
    } else if (delay_us > TRIAC_DELAY_MAX_US) {
        delay_us = TRIAC_DELAY_MAX_US;
    }

    /* Programa el instante exacto de disparo respecto al último ZC. */
    channel->fire_at_us = zc_time_us + TRIAC_FIXED_WAIT_US + delay_us;
    channel->armed = true;
    channel->pulsing = false;
    TEST_LOG("[PWM] %s schedule fire_at=%lu us (delay=%lu)\r\n",
             channel->name,
             channel->fire_at_us,
             delay_us);
}

static void triac_update_channel(triac_channel_t *channel, bool enabled)
{
    uint32_t now_us = app_get_time_us();

    if (!enabled) {
        channel->armed = false;
        channel->pulsing = false;
        HAL_GPIO_WritePin(channel->port, channel->pin, GPIO_PIN_RESET);
        return;
    }

    if (channel->armed) {
        /* Espera activa corta para mejorar precisión temporal de disparo. */
        uint32_t remaining_us = (uint32_t)(channel->fire_at_us - now_us);
        if (!time_reached(now_us, channel->fire_at_us) && remaining_us <= WAIT_SPIN_US) {
            while (!time_reached(app_get_time_us(), channel->fire_at_us)) {
            }
            now_us = app_get_time_us();
        }

        if (time_reached(now_us, channel->fire_at_us)) {
            HAL_GPIO_WritePin(channel->port, channel->pin, GPIO_PIN_SET);
            channel->pulse_off_us = channel->fire_at_us + TRIAC_PULSE_US;
            channel->armed = false;
            channel->pulsing = true;
            TEST_LOG("[PWM] %s FIRE ON at=%lu us OFF_at=%lu us\r\n",
                     channel->name,
                     now_us,
                     channel->pulse_off_us);
        }
    }

    if (channel->pulsing && time_reached(now_us, channel->pulse_off_us)) {
        HAL_GPIO_WritePin(channel->port, channel->pin, GPIO_PIN_RESET);
        channel->pulsing = false;
        TEST_LOG("[PWM] %s FIRE OFF at=%lu us\r\n", channel->name, now_us);
    }
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
