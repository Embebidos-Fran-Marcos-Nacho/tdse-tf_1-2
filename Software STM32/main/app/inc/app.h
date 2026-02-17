#ifndef APP_INC_APP_H_
#define APP_INC_APP_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdbool.h>
#include <stdint.h>

/********************** macros ***********************************************/

#define TEST_0 (0)
#define TEST_1 (1)
#define TEST_2 (2)

#define TEST_X (TEST_0)

/* Modo de prueba general para trazas por LOGGER_LOG.
 * 0 = desactivado (normal)
 * 1 = activado (debug de recorrido de tareas/estados/eventos)
 */
#define APP_TEST_MODE (0)

/* Simulación de cruce por cero para pruebas sin cableado externo.
 * Solo se usa si APP_TEST_MODE == 1.
 * 1 = genera eventos ZCD periódicos por software (100 Hz)
 * 0 = usa únicamente la interrupción real de ZCD (PC2)
 */
#define APP_TEST_SIMULATE_ZC (0)

/* Señal cuadrada de prueba en pin libre (PC8).
 * 1 = genera 100 Hz (toggle cada 5 ms)
 * 0 = pin fijo en bajo
 */
#define APP_TEST_WAVE_100HZ_PIN (0)

/* Verbosidad de trazas PWM en modo test.
 * 1 = imprime eventos de ZC/FIRE (muy ruidoso, puede distorsionar WCET)
 * 0 = silencia esos eventos para medir tiempos de scheduler/tareas
 */
#define APP_TEST_PWM_VERBOSE_LOGS (0)

/* Verbosidad de trazas ADC/SYS/BT en modo test.
 * Recomendado en 0 para medir WCET real de tareas (sin costo de UART por logs).
 */
#define APP_TEST_ADC_VERBOSE_LOGS (0)
#define APP_TEST_SYS_VERBOSE_LOGS (0)
#define APP_TEST_BT_VERBOSE_LOGS  (0)

/* Verificación simple de vida del módulo BT al iniciar (USART1).
 * Envía "AT" (sin CR/LF) y espera "OK".
 * APP_BT_AT_PROBE_STRICT:
 *   1 -> si falla, entra a ST_FAULT
 *   0 -> si falla, solo loguea y continúa
 */
#define APP_BT_AT_PROBE_ON_INIT (0)
#define APP_BT_AT_PROBE_STRICT  (0)
/* Período de telemetría BT en modo operación normal (ms). */
#define APP_BT_TELEMETRY_PERIOD_MS (50u)

/* Temporización de control por cruce por cero (50 Hz -> 10 ms por semiciclo). */
#define APP_ZC_HALF_CYCLE_US       (10000u)
#define APP_TRIAC_FIXED_WAIT_US    (700u)
#define APP_TRIAC_PULSE_US         (1000u)
#define APP_FAN_DIM_DELAY_MIN_US   (100u)
#define APP_FAN_DIM_DELAY_MAX_US   (7500u)

/* Failsafe de cruce por cero:
 * si no llegan eventos ZCD por hardware, genera eventos cada 10 ms.
 */
#define APP_ZC_FAILSAFE_ENABLE      (0)
#define APP_ZC_FAILSAFE_TIMEOUT_MS  (30u)
#define APP_ZC_FAILSAFE_PERIOD_MS   (10u)

/* Rango mínimo aceptable de calibración ADC para usar escala min/max. */
#define APP_ADC_CALIB_MIN_SPAN      (200u)
/* Deadband para generar evento de cambio de potenciómetro (en %).
 * Evita oscilaciones 99<->100 por ruido ADC.
 */
#define APP_ADC_PERCENT_EVENT_DEADBAND (2u)

/* Estrategia ante fallo al guardar en flash:
 * 1 -> entra en ST_FAULT
 * 0 -> sigue operando y solo informa por log
 */
#define APP_FLASH_STORE_STRICT      (1)

/* Perfilado temporal limpio (scheduler/tareas).
 * Cuando está en 1, emite una única línea por ventana con:
 * - Cavg y WCET de ventana por tarea
 * - uso promedio y pico de CPU
 * - overruns y backlog de ticks
 */
#define APP_PROFILE_ENABLE          (0)
#define APP_PROFILE_LOG_PERIOD_MS   (1000u)
#define APP_PROFILE_TASK_PERIOD_US  (1000u)

/********************** typedef **********************************************/

typedef struct {
	volatile bool zc_event_pending;
	volatile uint32_t zc_timestamp_us;
	volatile uint32_t last_zc_timestamp_us;
	volatile uint16_t zc_period_us;

	uint16_t adc_raw;
	uint16_t fan_delay_us;
	uint8_t adc_percent;
	uint16_t adc_calib_min;
	uint16_t adc_calib_max;
	bool adc_calib_valid;
	bool fan_enabled;
	bool light_enabled;
	uint8_t dip_value;
	bool bt_enabled;
	bool buzzer_enabled;
	bool led_enabled;

	bool ev_sys_pressed;
	bool ev_light_on_pressed;
	bool ev_light_off_pressed;
	bool ev_pote_changed;
	bool ev_send_bt_update_light;
	bool ev_send_bt_update_pote;

	bool fault_mode;
	bool cut_off_voltage;
	bool alarm_on;
	bool flash_save_light_request;
	bool flash_save_adc_calib_request;
} shared_data_type;

/********************** external data declaration ****************************/


/********************** external functions declaration ***********************/

void app_init(void);
void app_update(void);
uint32_t app_get_time_us(void);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_H_ */

/********************** end of file ******************************************/
