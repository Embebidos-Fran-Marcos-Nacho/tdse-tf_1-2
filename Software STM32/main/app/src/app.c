/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file   : app.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */


/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "app.h"
#include "board.h"
#include "task_adc.h"
#include "task_pwm.h"


/********************** macros and definitions *******************************/


#define G_APP_CNT_INI		0ul
#define G_APP_TICK_CNT_INI	0ul

#define TASK_X_WCET_INI		0ul

typedef struct {
	void (*task_init)(void *);		// Pointer to task (must be a
									// 'void (void *)' function)
	void (*task_update)(void *);	// Pointer to task (must be a
									// 'void (void *)' function)
	void *parameters;				// Pointer to parameters
} task_cfg_t;

typedef struct {
    uint32_t WCET;			// Worst-case execution time (microseconds)
} task_dta_t;

/********************** internal data declaration ****************************/


shared_data_type shared_data;

const task_cfg_t task_cfg_list[]	= {
		{task_adc_init, task_adc_update, &shared_data},
		{task_pwm_init,	task_pwm_update, &shared_data},
};

#define TASK_QTY (sizeof(task_cfg_list)/sizeof(task_cfg_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/


const char *p_sys	= " Bare Metal - Event-Triggered Systems (ETS)\n";
const char *p_app	= " ADC + PWM\n";


/********************** external data declaration *****************************/


uint32_t g_app_cnt;
uint32_t g_app_time_us;

volatile uint32_t g_app_tick_cnt;
task_dta_t task_dta_list[TASK_QTY];

/********************** external functions definition ************************/

void app_init(void)
{
	uint32_t index;

	/* Print out: Application Initialized */
	LOGGER_LOG("\n");
	LOGGER_LOG("%s is running - Tick [mS] = %lu\r\n", GET_NAME(app_init), HAL_GetTick());

	LOGGER_LOG(p_sys);
	LOGGER_LOG(p_app);

	g_app_cnt = G_APP_CNT_INI;

	/* Print out: Application execution counter */
	LOGGER_LOG(" %s = %lu\n", GET_NAME(g_app_cnt), g_app_cnt);

	/* Go through the task arrays */
	for (index = 0; TASK_QTY > index; index++)
	{

		/* Run task_x_init */
		(*task_cfg_list[index].task_init)(task_cfg_list[index].parameters);

		/* Init variables */
		task_dta_list[index].WCET = TASK_X_WCET_INI;
	}

	cycle_counter_init();
}

void app_update(void)
{

	uint32_t index;
	//uint32_t cycle_counter;
	uint32_t cycle_counter_time_us;

	/* Check if it's time to run tasks */
	if (G_APP_TICK_CNT_INI < g_app_tick_cnt)
    {

    	g_app_tick_cnt--;

    	/* Update App Counter */
    	g_app_cnt++;
    	g_app_time_us = 0;

		/* Print out: Application execution counter */
		//LOGGER_LOG(" %s = %lu\r\n", GET_NAME(g_app_cnt), g_app_cnt);

		/* Go through the task arrays */
		for (index = 0; TASK_QTY > index; index++)
		{

			cycle_counter_reset();

			/* Run task_x_update */
			(*task_cfg_list[index].task_update)(task_cfg_list[index].parameters);


			//cycle_counter = cycle_counter_get();
			cycle_counter_time_us = cycle_counter_time_us();

			/* Update variables */
			g_app_time_us += cycle_counter_time_us;

			if (task_dta_list[index].WCET < cycle_counter_time_us)
			{
				task_dta_list[index].WCET = cycle_counter_time_us;
			}


			/* Print out: Cycle Counter */
			//LOGGER_LOG(" %s: %lu - %s: %lu uS\r\n", GET_NAME(cycle_counter), cycle_counter, GET_NAME(cycle_counter_time_us), cycle_counter_time_us);
			//LOGGER_LOG(" %s: %lu uS\r\n", GET_NAME(g_app_time_us), g_app_time_us);
		}

		LOGGER_LOG("%d,%d\n", shared_data.pwm_active, shared_data.adc_value);
    }
}

/* Callbacks in C (https://www.geeksforgeeks.org/) */
/*
 * A callback is any executable code that is passed as an argument to another
 * code, which is expected to call back (execute) the argument at a given time.
 * In simple language, If a reference of a function is passed to another
 * function as an argument to call it, then it will be called a Callback
 * function.
 */

void HAL_SYSTICK_Callback(void)
{
	g_app_tick_cnt++;



	//HAL_GPIO_TogglePin(LED_A_PORT, LED_A_PIN);
}

/********************** end of file ******************************************/
