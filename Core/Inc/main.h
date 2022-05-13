/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#ifndef EJE_TEST
#define EJE_TEST

struct eje_str{//parametros fijos, levantados de la flash en el arranque o en el regrabado

	//double mm_actual;
	//double mm_goto;
	int64_t step_mm_actual;
	int64_t step_mm_goto;

	uint32_t v_max;
	uint32_t v_G1;
	uint32_t acc;
	uint32_t porcentaje_v_min; // de 0 a 100 porcentaje de v_max
	double step_mm;
	uint8_t invertir_direccion;
	uint8_t home_act;
	uint8_t home_dir;
	uint8_t home_prioridad;
	uint8_t home_sw_nromal_0_1;
	uint16_t home_mm_retroceso;
	uint16_t home_porcentaje_vel;
	double home_ubic_mm;

	uint16_t dir_pin;
	uint16_t step_pin;
	uint16_t home_pin;



};
#endif
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Home_C_Pin GPIO_PIN_0
#define Home_C_GPIO_Port GPIOA
#define Home_B_Pin GPIO_PIN_1
#define Home_B_GPIO_Port GPIOA
#define Home_Z_Pin GPIO_PIN_2
#define Home_Z_GPIO_Port GPIOA
#define Home_Y_Pin GPIO_PIN_3
#define Home_Y_GPIO_Port GPIOA
#define Home_X_Pin GPIO_PIN_4
#define Home_X_GPIO_Port GPIOA
#define E_Stop_Pin GPIO_PIN_5
#define E_Stop_GPIO_Port GPIOA
#define Luz_up_Pin GPIO_PIN_6
#define Luz_up_GPIO_Port GPIOA
#define Luz_bottom_Pin GPIO_PIN_7
#define Luz_bottom_GPIO_Port GPIOA
#define Step_X_Pin GPIO_PIN_0
#define Step_X_GPIO_Port GPIOB
#define Dir_X_Pin GPIO_PIN_1
#define Dir_X_GPIO_Port GPIOB
#define Step_Y_Pin GPIO_PIN_10
#define Step_Y_GPIO_Port GPIOB
#define Dir_Y_Pin GPIO_PIN_11
#define Dir_Y_GPIO_Port GPIOB
#define Step_Z_Pin GPIO_PIN_12
#define Step_Z_GPIO_Port GPIOB
#define Dir_Z_Pin GPIO_PIN_13
#define Dir_Z_GPIO_Port GPIOB
#define Step_C_Pin GPIO_PIN_14
#define Step_C_GPIO_Port GPIOB
#define Dir_C_Pin GPIO_PIN_15
#define Dir_C_GPIO_Port GPIOB
#define Step_B_Pin GPIO_PIN_3
#define Step_B_GPIO_Port GPIOB
#define Dir_B_Pin GPIO_PIN_4
#define Dir_B_GPIO_Port GPIOB
#define Enable_motors_Pin GPIO_PIN_5
#define Enable_motors_GPIO_Port GPIOB
#define Vacuum_out_Pin GPIO_PIN_6
#define Vacuum_out_GPIO_Port GPIOB
#define Sensing_vac_in_Pin GPIO_PIN_7
#define Sensing_vac_in_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
