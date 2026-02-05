#ifndef APP_INC_APP_H_
#define APP_INC_APP_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdbool.h>

/********************** macros ***********************************************/

#define TEST_0 (0)
#define TEST_1 (1)
#define TEST_2 (2)

#define TEST_X (TEST_0)

/********************** typedef **********************************************/

typedef struct {
	bool adc_end_of_conversion;
	uint16_t adc_value;
	uint16_t pwm_active;
} shared_data_type;

/********************** external data declaration ****************************/


/********************** external functions declaration ***********************/

void app_init(void);
void app_update(void);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_H_ */

/********************** end of file ******************************************/
