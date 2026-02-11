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
#define APP_TEST_MODE (1)

/* Simulación de cruce por cero para pruebas sin cableado externo.
 * Solo se usa si APP_TEST_MODE == 1.
 * 1 = genera eventos ZCD periódicos por software (100 Hz)
 * 0 = usa únicamente la interrupción real de ZCD (PC2)
 */
#define APP_TEST_SIMULATE_ZC (1)

/* Señal cuadrada de prueba en pin libre (PC8).
 * 1 = genera 100 Hz (toggle cada 5 ms)
 * 0 = pin fijo en bajo
 */
#define APP_TEST_WAVE_100HZ_PIN (1)

/* Verificación simple de vida del módulo BT al iniciar (USART1).
 * Envía "AT" (sin CR/LF) y espera "OK".
 * APP_BT_AT_PROBE_STRICT:
 *   1 -> si falla, entra a ST_FAULT
 *   0 -> si falla, solo loguea y continúa
 */
#define APP_BT_AT_PROBE_ON_INIT (1)
#define APP_BT_AT_PROBE_STRICT  (0)

/********************** typedef **********************************************/

typedef struct {
	volatile bool zc_event_pending;
	volatile uint32_t zc_timestamp_us;
	volatile uint32_t last_zc_timestamp_us;

	uint16_t adc_raw;
	uint16_t fan_delay_us;
	uint8_t adc_percent;
	bool fan_enabled;
	bool light_enabled;
	uint8_t dip_value;
	bool bt_enabled;
	bool buzzer_enabled;
	bool led_enabled;

	bool ev_sys_pressed;
	bool ev_pote_changed;
	bool ev_send_bt_update_light;
	bool ev_send_bt_update_pote;

	bool fault_mode;
	bool cut_off_voltage;
	bool alarm_on;
	bool flash_save_light_request;
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
