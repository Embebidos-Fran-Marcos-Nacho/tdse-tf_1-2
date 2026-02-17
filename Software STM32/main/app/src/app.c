/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 */

/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "app.h"
#include "task_adc.h"
#include "task_system.h"
#include "task_pwm.h"

#if APP_TEST_MODE
#define TEST_LOG(...) LOGGER_LOG(__VA_ARGS__)
#else
#define TEST_LOG(...)
#endif

/********************** macros and definitions *******************************/
#define G_APP_CNT_INI       0ul
#define G_APP_TICK_CNT_INI  0ul
#define TASK_X_WCET_INI     0ul
#define ZC_HOLDOFF_US       2000ul

typedef struct {
    void (*task_init)(void *);
    void (*task_update)(void *);
    void *parameters;
} task_cfg_t;

typedef struct {
    uint32_t WCET;
} task_dta_t;

/********************** internal data declaration ****************************/
shared_data_type shared_data;

const task_cfg_t task_cfg_list[] = {
    {task_adc_init, task_adc_update, &shared_data},
    {task_system_init, task_system_update, &shared_data},
    {task_pwm_init, task_pwm_update, &shared_data},
};

#define TASK_QTY (sizeof(task_cfg_list) / sizeof(task_cfg_t))

/********************** external data declaration *****************************/
uint32_t g_app_cnt;
uint32_t g_app_time_us;

volatile uint32_t g_app_tick_cnt;
task_dta_t task_dta_list[TASK_QTY];

/********************** external functions definition ************************/
void app_init(void)
{
    uint32_t index;

    TEST_LOG("\n");
    TEST_LOG("%s is running - Tick [mS] = %lu\r\n", GET_NAME(app_init), HAL_GetTick());

    g_app_cnt = G_APP_CNT_INI;
    g_app_tick_cnt = G_APP_TICK_CNT_INI;

    shared_data.zc_event_pending = false;
    shared_data.zc_timestamp_us = 0u;
    shared_data.last_zc_timestamp_us = 0u;
    shared_data.zc_period_us = APP_ZC_HALF_CYCLE_US;
    shared_data.adc_raw = 0u;
    shared_data.fan_delay_us = 0u;
    shared_data.adc_percent = 0u;
    shared_data.adc_calib_min = 0u;
    shared_data.adc_calib_max = 4095u;
    shared_data.adc_calib_valid = false;
    shared_data.fan_enabled = true;
    shared_data.light_enabled = false;
    shared_data.dip_value = 0u;
    shared_data.bt_enabled = false;
    shared_data.buzzer_enabled = false;
    shared_data.led_enabled = false;
    shared_data.ev_sys_pressed = false;
    shared_data.ev_light_on_pressed = false;
    shared_data.ev_light_off_pressed = false;
    shared_data.ev_pote_changed = false;
    shared_data.ev_send_bt_update_light = false;
    shared_data.ev_send_bt_update_pote = false;
    shared_data.fault_mode = false;
    shared_data.cut_off_voltage = false;
    shared_data.alarm_on = false;
    shared_data.flash_save_light_request = false;
    shared_data.flash_save_adc_calib_request = false;

    /* Inicializa todas las tareas registradas en el scheduler. */
    for (index = 0; TASK_QTY > index; index++) {
        (*task_cfg_list[index].task_init)(task_cfg_list[index].parameters);
        task_dta_list[index].WCET = TASK_X_WCET_INI;
    }

    cycle_counter_init();
}

void app_update(void)
{
    uint32_t index;
    static uint32_t last_health_log_ms = 0u;
#if APP_TEST_MODE && APP_TEST_SIMULATE_ZC
    static uint32_t last_simulated_zc_ms = 0u;
#endif
#if APP_ZC_FAILSAFE_ENABLE
    static uint32_t last_failsafe_zc_ms = 0u;
#endif
#if APP_TEST_MODE && APP_TEST_WAVE_100HZ_PIN
    static uint32_t last_wave_toggle_ms = 0u;
    static bool wave_state = false;
#endif

    /* Ejecuta un ciclo del scheduler cada tick de SysTick (1 ms). */
    if (G_APP_TICK_CNT_INI < g_app_tick_cnt) {
        g_app_tick_cnt--;
        g_app_cnt++;
        g_app_time_us = 0u;

#if APP_TEST_MODE && APP_TEST_SIMULATE_ZC
        /* Simula cruces por cero a 100 Hz (cada 10 ms) para pruebas en banco. */
        if ((HAL_GetTick() - last_simulated_zc_ms) >= 10u) {
            last_simulated_zc_ms = HAL_GetTick();
            shared_data.last_zc_timestamp_us = shared_data.zc_timestamp_us;
            shared_data.zc_timestamp_us = app_get_time_us();
            shared_data.zc_period_us = APP_ZC_HALF_CYCLE_US;
            shared_data.zc_event_pending = true;
            task_pwm_on_zero_crossing_isr(&shared_data);
        }
#endif

#if APP_ZC_FAILSAFE_ENABLE
        {
            uint32_t now_ms = HAL_GetTick();
            uint32_t last_zc_ms = shared_data.last_zc_timestamp_us / 1000u;

            /* Si no hay ZCD real por un tiempo, fuerza ZC periódico para mantener control. */
            if ((now_ms - last_zc_ms) >= APP_ZC_FAILSAFE_TIMEOUT_MS) {
                if ((now_ms - last_failsafe_zc_ms) >= APP_ZC_FAILSAFE_PERIOD_MS) {
                    last_failsafe_zc_ms = now_ms;
                    shared_data.last_zc_timestamp_us = shared_data.zc_timestamp_us;
                    shared_data.zc_timestamp_us = app_get_time_us();
                    shared_data.zc_period_us = APP_ZC_HALF_CYCLE_US;
                    shared_data.zc_event_pending = true;
                    task_pwm_on_zero_crossing_isr(&shared_data);
                }
            }
        }
#endif

#if APP_TEST_MODE && APP_TEST_WAVE_100HZ_PIN
        /* Genera onda cuadrada de 100 Hz en PC8 (toggle cada 5 ms). */
        if ((HAL_GetTick() - last_wave_toggle_ms) >= 5u) {
            last_wave_toggle_ms = HAL_GetTick();
            wave_state = !wave_state;
            HAL_GPIO_WritePin(TEST_WAVE_100HZ_GPIO_Port,
                              TEST_WAVE_100HZ_Pin,
                              wave_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }
#else
        HAL_GPIO_WritePin(TEST_WAVE_100HZ_GPIO_Port, TEST_WAVE_100HZ_Pin, GPIO_PIN_RESET);
#endif

        /* Recorre las tareas en orden fijo: sensor -> sistema -> actuador. */
        for (index = 0; TASK_QTY > index; index++) {
            uint32_t start_cycles = DWT->CYCCNT;
            (*task_cfg_list[index].task_update)(task_cfg_list[index].parameters);
            uint32_t elapsed_us = (DWT->CYCCNT - start_cycles) / cycles_per_us;

            g_app_time_us += elapsed_us;

            if (task_dta_list[index].WCET < elapsed_us) {
                task_dta_list[index].WCET = elapsed_us;
            }
        }

        if ((HAL_GetTick() - last_health_log_ms) >= 1000u) {
            last_health_log_ms = HAL_GetTick();
            TEST_LOG("[APP] t=%lu ms cnt=%lu runtime=%lu us wcet={%lu,%lu,%lu}\r\n",
                     HAL_GetTick(),
                     g_app_cnt,
                     g_app_time_us,
                     task_dta_list[0].WCET,
                     task_dta_list[1].WCET,
                     task_dta_list[2].WCET);
        }
    }
}

uint32_t app_get_time_us(void)
{
    return (DWT->CYCCNT / cycles_per_us);
}

void HAL_SYSTICK_Callback(void)
{
    g_app_tick_cnt++;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ZCD_INT_Pin) {
        uint32_t now_us = app_get_time_us();
        uint32_t zc_period_us = (uint32_t)(now_us - shared_data.last_zc_timestamp_us);

        /* Filtro anti-rebote/ruido del detector de cruce por cero. */
        if (zc_period_us > ZC_HOLDOFF_US) {
            /* Este hardware entrega ZC en cada cruce (semiciclo, ~100 Hz @ 50 Hz de red). */
            if ((zc_period_us >= 7000u) && (zc_period_us <= 13000u)) {
                shared_data.zc_period_us = (uint16_t)zc_period_us;
            }
            shared_data.last_zc_timestamp_us = now_us;
            shared_data.zc_timestamp_us = now_us;
            shared_data.zc_event_pending = true;
            task_pwm_on_zero_crossing_isr(&shared_data);
        }
    }
}

/********************** end of file ******************************************/
