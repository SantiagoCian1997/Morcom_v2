/*
 * home_m.h
 *
 *  Created on: May, 2022
 *      Author: Cian, Santiago Jose -- santiagocian97@gmail.com
 *              https://github.com/SantiagoCian1997/Morcom_v2
 */

#ifndef HOME_M_H
#define HOME_M_H

#include "stm32f1xx_hal.h"
#include "morcom_v2.h"
#include "stdio.h"
#include "run_m.h"

/*
********************************************************************************
*                   PROTOTYPES
********************************************************************************
*/

void home_sequence(void);
void home_sequence_axis(struct eje_str *);

#endif /* HOME_M_H */

/*** end of file ***/
