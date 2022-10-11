/*
 * run_m.h
 *
 *  Created on: May, 2022
 *      Author: Cian, Santiago Jose -- santiagocian97@gmail.com
 *              https://github.com/SantiagoCian1997/Morcom_v2
 */

#ifndef RUN_M_H
#define RUN_M_H

#include "morcom_v2.h"
#include "stm32f1xx_hal.h"
#include "main.h"
#include "stdio.h"

/*
********************************************************************************
*                   DEFINE CONSTANTS
********************************************************************************
*/

#define F_BASE_Hz 107000

/*
********************************************************************************
*                   PROTOTYPES
********************************************************************************
*/

void 		run_motors(void);
void 		axes_init(struct eje_str *);
uint8_t 	check_limit_switch(struct eje_str *);
void 		e_stop_routine(void);

void 		run_1_motor(struct eje_str *);
void		run_2_motor(struct eje_str *, struct eje_str *);
void 		run_3_motor(struct eje_str *, struct eje_str *, struct eje_str *);
void 		run_4_motor(struct eje_str *, struct eje_str *, struct eje_str *, struct eje_str *);
void 		run_5_motor(struct eje_str *, struct eje_str *, struct eje_str *, struct eje_str *, struct eje_str *);

#endif /* RUN_M_H */

/*** end of file ***/
