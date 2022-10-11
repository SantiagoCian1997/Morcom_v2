/*
 * morcom_v2.c
 *
 *   Created on: May, 2022
 *       Author: Cian, Santiago Jose -- santiagocian97@gmail.com
 *               https://github.com/SantiagoCian1997/Morcom_v2
 *
 * 	Description:
 */

#include "home_m.h"


extern struct eje_str *p_ejes[];

/* Description:
 *			check the priorities of the homing sequence of each axis and call the homing sequence
 * Parameters:
 *			none
 * Return:
 *
 * Notes:
 *
 */
void home_sequence(){
	struct eje_str *sec_home_ejes[5];
	uint8_t cont = 0;
	for(uint8_t i=0; i<255; i++){
		for(uint8_t ej=0; ej<N_MOTORS; ej++){
			if((*p_ejes[ej]).home_prioridad == i && (*p_ejes[ej]).home_act == 1) sec_home_ejes[cont++] = p_ejes[ej];
		}
	}


	for(uint8_t ej=0; ej<cont; ej++) home_sequence_axis(sec_home_ejes[ej]);
}

/* Description:
 *			implement the homing sequence
 * Parameters:
 *			struct eje_str *eje: axis pointer
 * Return:
 *
 * Notes:
 *
 */
void home_sequence_axis(struct eje_str *eje){
	uint64_t f_desp=(( (*eje).v_max*(*eje).step_mm )*(*eje).home_porcentaje_vel);				//despulso al % de la velocidad maxima del eje
	f_desp /= 100;
	f_desp = 1334000/f_desp;											//obtengo el retardo en base a el for implementado

	printf("Buscando eje %c\n",(*eje).char_name);

	HAL_GPIO_WritePin(GPIOB, (*eje).dir_pin, (*eje).home_dir);
	if((*eje).invertir_direccion) HAL_GPIO_TogglePin(GPIOB, (*eje).dir_pin);

	while(!HAL_GPIO_ReadPin(GPIOA, (*eje).home_pin)){			//busco hasta pulsar
		HAL_GPIO_TogglePin(GPIOB, (*eje).step_pin);
		for(uint32_t h=0; h<f_desp; h++){//delay
			   	asm("NOP");
			   	asm("NOP");
		}
	}

	HAL_GPIO_TogglePin(GPIOB, (*eje).dir_pin);
	for(uint32_t h=0; h<2000000; h++){//delay
	   	asm("NOP");
	   	asm("NOP");
	}

	f_desp *= 4;
	while(HAL_GPIO_ReadPin(GPIOA, (*eje).home_pin)){			//busco mas lento hasta despulsar
		HAL_GPIO_TogglePin(GPIOB, (*eje).step_pin);
		for(uint32_t h=0; h<f_desp; h++){//delay
			   	asm("NOP");
			   	asm("NOP");
		}
	}

	for(uint32_t h=0; h<2000000; h++){//delay
	   	asm("NOP");
	   	asm("NOP");
	}

	(*eje).v_mov = (*eje).v_max;
	(*eje).steps_actual = 0;
	(*eje).steps_goto = (*eje).home_mm_retroceso*(*eje).step_mm;
	if((*eje).home_dir==1) (*eje).steps_goto =- (*eje).steps_goto;

	run_motors();

	(*eje).steps_goto = (*eje).home_ubic_mm*(*eje).step_mm;
	(*eje).steps_actual = (*eje).steps_goto;
}

/*** end of file ***/
