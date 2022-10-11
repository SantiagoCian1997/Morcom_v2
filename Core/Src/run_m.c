/*
 * morcom_v2.c
 *
 *   Created on: May, 2022
 *       Author: Cian, Santiago Jose -- santiagocian97@gmail.com
 *               https://github.com/SantiagoCian1997/Morcom_v2
 *
 * 	Description:
 */

#include "run_m.h"

extern struct eje_str *p_ejes[];

/* Description:
 *			checks the axes to be moved, initializes them and generates the movement
 * Parameters:
 *			none
 * Return:
 *
 * Notes:
 *
 */
void run_motors(){
	struct eje_str *p_move[N_MOTORS];
	uint8_t i = 0;

	for(uint8_t ej=0; ej<5; ej++) if((*p_ejes[ej]).steps_actual != (*p_ejes[ej]).steps_goto) p_move[i++] = p_ejes[ej];

	for(uint8_t j=0; j<i; j++) axes_init(p_move[j]);

	switch (i){
		case 1: run_1_motor(p_move[0]); break;
		case 2: run_2_motor(p_move[0], p_move[1]); break;
		case 3: run_3_motor(p_move[0], p_move[1], p_move[2]); break;
		case 4: run_4_motor(p_move[0], p_move[1], p_move[2], p_move[3]); break;
		case 5: run_5_motor(p_move[0], p_move[1], p_move[2], p_move[3], p_move[4]); break;
		default: {}break;
	}
}


/* Description:
 *			initialize the axis and set parameters for implementation the movement
 * Parameters:
 *			struct eje_str *eje: axis pointer to initialize
 * Return:
 *
 * Notes:
 *
 */
void axes_init(struct eje_str *eje){

	if((*eje).steps_actual < (*eje).steps_goto) {//movimiento
		GPIOB -> ODR |= (*eje).dir_pin;//set//set estado pin direccion
		(*eje).distancia_v3 = (*eje).steps_goto - (*eje).steps_actual;
	}
	if((*eje).steps_actual > (*eje).steps_goto) {
		GPIOB -> ODR &= ~(*eje).dir_pin;//reset//set estado pin direccion
		(*eje).distancia_v3 = (*eje).steps_actual - (*eje).steps_goto;
	}
	if((*eje).steps_actual == (*eje).steps_goto)  (*eje).distancia_v3 = 0;
	if((*eje).invertir_direccion) HAL_GPIO_TogglePin(GPIOB, (*eje).dir_pin);

	(*eje).f_max_v3 =  (*eje).v_mov*(*eje).step_mm;									 	//falta interpolacion
	(*eje).f_min_v3 =  ((*eje).f_max_v3*(*eje).porcentaje_v_min)/100;				//falta interpolacion
	(*eje).f_acc_v3 =  ((*eje).v_mov*(*eje).acc*(*eje).step_mm);					//falta interpolacion
	(*eje).f_acc_v3 /= (*eje).v_max;

	if((*eje).f_acc_v3 == 0) (*eje).f_acc_v3 = 1;
	if((*eje).f_max_v3 == 0) (*eje).f_max_v3 = 1;
	if((*eje).f_min_v3 == 0) (*eje).f_min_v3 = 1;


	(*eje).delta_v3			= 0;
	(*eje).N_loop_acc_v3	= 0;
	(*eje).N_loop_desacc_v3	= 0;
	(*eje).f_actual_v3		= 0;
	(*eje).f_cresta_v3		= 0;
	(*eje).error_v3			= 0;

	if((*eje).f_acc_v3*(*eje).distancia_v3 > (*eje).f_max_v3*(*eje).f_max_v3){
		(*eje).step_1_v3 = (*eje).f_max_v3*(*eje).f_max_v3 / ((*eje).f_acc_v3*2);
		(*eje).step_3_v3 = (*eje).distancia_v3 - (*eje).step_1_v3;
	}else{
		(*eje).step_1_v3 = (*eje).distancia_v3/2;
		(*eje).step_3_v3 = (*eje).distancia_v3 - (*eje).step_1_v3;
	}
	(*eje).step_1_v3 = (*eje).distancia_v3 - (*eje).step_1_v3;//cambio la refererncia porque voy descontando
	(*eje).step_3_v3 = (*eje).distancia_v3 - (*eje).step_3_v3;

	printf("%c: %10i->%10i, dis=%i, f_max: %i, f_acc: %i, f_min: %i\n",(*eje).char_name,(int)(*eje).steps_actual,(int)(*eje).steps_goto,(int)(*eje).distancia_v3,(int)(*eje).f_max_v3,(int)(*eje).f_acc_v3,(int)(*eje).f_min_v3);

}

/* Description:
 *			check and releases the switch of an axis
 * Parameters:
 *			struct eje_str *eje: axis pinter
 * Return:
 *
 * Notes:
 *
 */
uint8_t check_limit_switch(struct eje_str *eje){
	if((*eje).home_act == 0)	return(0);
	uint64_t f_desp=((*eje).v_max*(*eje).step_mm)/10;				//despulso al 10% de la velocidad maxima del eje
	f_desp=1334000/f_desp;										//obtengo el retardo en base a el for implementado
	if(HAL_GPIO_ReadPin(GPIOA, (*eje).home_pin)){
		printf("Error eje: %c\n",(*eje).char_name);
		HAL_GPIO_TogglePin(GPIOB, (*eje).dir_pin);//invierto la direccion
		while(HAL_GPIO_ReadPin(GPIOA, (*eje).home_pin)){
			HAL_GPIO_TogglePin(GPIOB, (*eje).step_pin);
			for(uint32_t h=0; h<f_desp; h++){//delay
				   	asm("NOP");
				   	asm("NOP");
			}
		}
		(*eje).distancia_v3 = 0;
	}
	return(0);
}

/* Description:
 *			check which of the axes triggered the emergency stop and releases the switch
 * Parameters:
 *			none
 * Return:
 *			0
 * Notes:
 *
 */
void e_stop_routine(){
	printf("E Stop!\n");
	for(uint32_t h=0; h<1500000; h++){//delay
		   	asm("NOP");
		   	asm("NOP");
	}
	for(uint8_t ej=0; ej<N_MOTORS; ej++) check_limit_switch(p_ejes[ej]);
}

/* Description:
 *			generates 1-axis motion
 * Parameters:
 *			struct eje_str *ej0: axis pointer 1
 * Return:
 *
 * Notes:
 *
 */
void run_1_motor(struct eje_str *ej0){
	uint32_t factor_1=F_BASE_Hz;

	while ((*ej0).distancia_v3>0){
		if(GPIOA -> IDR & E_Stop_Pin) e_stop_routine();

		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset



		  if((*ej0).distancia_v3<(*ej0).step_3_v3){			//CASO 3
			  (*ej0).f_actual_v3=(*ej0).f_cresta_v3-((*ej0).N_loop_desacc_v3*(*ej0).f_acc_v3)/factor_1;
			  if((*ej0).f_actual_v3<=(*ej0).f_min_v3){		//CASO 4
				  (*ej0).f_actual_v3=(*ej0).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej0).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej0).distancia_v3>=(*ej0).step_1_v3){				//CASO 1
			 			 (*ej0).f_actual_v3=((*ej0).N_loop_acc_v3*(*ej0).f_acc_v3)/factor_1;
			 			 (*ej0).N_loop_acc_v3++;
			 			 (*ej0).f_cresta_v3=(*ej0).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej0).f_actual_v3=(*ej0).f_max_v3;

				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3>factor_1 && (*ej0).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej0).step_pin;
			(*ej0).error_v3=(*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3-factor_1;
			if((*ej0).error_v3>factor_1) (*ej0).error_v3=0;
			else{
				(*ej0).null_operation_v3++;
			}
			(*ej0).delta_v3=0;
		   	(*ej0).distancia_v3--;
		}else{
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej0).step_pin;
		}
		(*ej0).delta_v3++;
		GPIOB -> ODR &= ~(*ej0).step_pin;

	}


	GPIOB -> ODR &= ~(*ej0).step_pin;
	(*ej0).steps_actual=(*ej0).steps_goto;
}

/* Description:
 *			generates 1-axis motion
 * Parameters:
 *			struct eje_str *ej0: axis pointer 1
 *			struct eje_str *ej1: axis pointer 2
 * Return:
 *
 * Notes:
 *
 */
void run_2_motor(struct eje_str *ej0, struct eje_str *ej1){
	uint32_t factor_1=F_BASE_Hz/2;

	while ((*ej0).distancia_v3+(*ej1).distancia_v3>0){
		if(GPIOA -> IDR & E_Stop_Pin) e_stop_routine();

		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset

		  if((*ej0).distancia_v3<(*ej0).step_3_v3){			//CASO 3
			  (*ej0).f_actual_v3=(*ej0).f_cresta_v3-((*ej0).N_loop_desacc_v3*(*ej0).f_acc_v3)/factor_1;
			  if((*ej0).f_actual_v3<=(*ej0).f_min_v3){		//CASO 4
				  (*ej0).f_actual_v3=(*ej0).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej0).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej0).distancia_v3>=(*ej0).step_1_v3){				//CASO 1
			 			 (*ej0).f_actual_v3=((*ej0).N_loop_acc_v3*(*ej0).f_acc_v3)/factor_1;
			 			 (*ej0).N_loop_acc_v3++;
			 			 (*ej0).f_cresta_v3=(*ej0).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej0).f_actual_v3=(*ej0).f_max_v3;

				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3>factor_1 && (*ej0).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej0).step_pin;
			(*ej0).error_v3=(*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3-factor_1;
			if((*ej0).error_v3>factor_1) (*ej0).error_v3=0;
			else{
				(*ej0).null_operation_v3++;
			}
			(*ej0).delta_v3=0;
		   	(*ej0).distancia_v3--;
		}else{
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej0).step_pin;
		}
		(*ej0).delta_v3++;
		GPIOB -> ODR &= ~(*ej0).step_pin;

		  if((*ej1).distancia_v3<(*ej1).step_3_v3){			//CASO 3
			  (*ej1).f_actual_v3=(*ej1).f_cresta_v3-((*ej1).N_loop_desacc_v3*(*ej1).f_acc_v3)/factor_1;
			  if((*ej1).f_actual_v3<=(*ej1).f_min_v3){		//CASO 4
				  (*ej1).f_actual_v3=(*ej1).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej1).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej1).distancia_v3>=(*ej1).step_1_v3){				//CASO 1
			 			 (*ej1).f_actual_v3=((*ej1).N_loop_acc_v3*(*ej1).f_acc_v3)/factor_1;
			 			 (*ej1).N_loop_acc_v3++;
			 			 (*ej1).f_cresta_v3=(*ej1).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej1).f_actual_v3=(*ej1).f_max_v3;

				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3>factor_1 && (*ej1).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej1).step_pin;
			(*ej1).error_v3=(*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3-factor_1;
			if((*ej1).error_v3>factor_1) (*ej1).error_v3=0;
			else{
				(*ej1).null_operation_v3++;
			}
			(*ej1).delta_v3=0;
		   	(*ej1).distancia_v3--;
		}else{
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej1).step_pin;
		}
		(*ej1).delta_v3++;
		GPIOB -> ODR &= ~(*ej1).step_pin;

	}


	GPIOB -> ODR &= ~(*ej0).step_pin;
	GPIOB -> ODR &= ~(*ej1).step_pin;
	(*ej0).steps_actual=(*ej0).steps_goto;
	(*ej1).steps_actual=(*ej1).steps_goto;
}


/* Description:
 *			generates 1-axis motion
 * Parameters:
 *			struct eje_str *ej0: axis pointer 1
 *			struct eje_str *ej1: axis pointer 2
 *			struct eje_str *ej2: axis pointer 3
 * Return:
 *
 * Notes:
 *
 */
void run_3_motor(struct eje_str *ej0, struct eje_str *ej1, struct eje_str *ej2){
	uint32_t factor_1=F_BASE_Hz/3;

	while ((*ej0).distancia_v3+(*ej1).distancia_v3+(*ej2).distancia_v3>0){
		if(GPIOA -> IDR & E_Stop_Pin) e_stop_routine();

		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset

		  if((*ej0).distancia_v3<(*ej0).step_3_v3){			//CASO 3
			  (*ej0).f_actual_v3=(*ej0).f_cresta_v3-((*ej0).N_loop_desacc_v3*(*ej0).f_acc_v3)/factor_1;
			  if((*ej0).f_actual_v3<=(*ej0).f_min_v3){		//CASO 4
				  (*ej0).f_actual_v3=(*ej0).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej0).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej0).distancia_v3>=(*ej0).step_1_v3){				//CASO 1
			 			 (*ej0).f_actual_v3=((*ej0).N_loop_acc_v3*(*ej0).f_acc_v3)/factor_1;
			 			 (*ej0).N_loop_acc_v3++;
			 			 (*ej0).f_cresta_v3=(*ej0).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej0).f_actual_v3=(*ej0).f_max_v3;

				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3>factor_1 && (*ej0).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej0).step_pin;
			(*ej0).error_v3=(*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3-factor_1;
			if((*ej0).error_v3>factor_1) (*ej0).error_v3=0;
			else{
				(*ej0).null_operation_v3++;
			}
			(*ej0).delta_v3=0;
		   	(*ej0).distancia_v3--;
		}else{//else del if anterior
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej0).step_pin;
		}
		(*ej0).delta_v3++;
		GPIOB -> ODR &= ~(*ej0).step_pin;

		  if((*ej1).distancia_v3<(*ej1).step_3_v3){			//CASO 3
			  (*ej1).f_actual_v3=(*ej1).f_cresta_v3-((*ej1).N_loop_desacc_v3*(*ej1).f_acc_v3)/factor_1;
			  if((*ej1).f_actual_v3<=(*ej1).f_min_v3){		//CASO 4
				  (*ej1).f_actual_v3=(*ej1).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej1).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej1).distancia_v3>=(*ej1).step_1_v3){				//CASO 1
			 			 (*ej1).f_actual_v3=((*ej1).N_loop_acc_v3*(*ej1).f_acc_v3)/factor_1;
			 			 (*ej1).N_loop_acc_v3++;
			 			 (*ej1).f_cresta_v3=(*ej1).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej1).f_actual_v3=(*ej1).f_max_v3;

				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3>factor_1 && (*ej1).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej1).step_pin;
			(*ej1).error_v3=(*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3-factor_1;
			if((*ej1).error_v3>factor_1) (*ej1).error_v3=0;
			else{
				(*ej1).null_operation_v3++;
			}
			(*ej1).delta_v3=0;
		   	(*ej1).distancia_v3--;
		}else{
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej1).step_pin;
		}
		(*ej1).delta_v3++;
		GPIOB -> ODR &= ~(*ej1).step_pin;

		  if((*ej2).distancia_v3<(*ej2).step_3_v3){			//CASO 3
			  (*ej2).f_actual_v3=(*ej2).f_cresta_v3-((*ej2).N_loop_desacc_v3*(*ej2).f_acc_v3)/factor_1;
			  if((*ej2).f_actual_v3<=(*ej2).f_min_v3){		//CASO 4
				  (*ej2).f_actual_v3=(*ej2).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej2).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej2).distancia_v3>=(*ej2).step_1_v3){				//CASO 1
			 			 (*ej2).f_actual_v3=((*ej2).N_loop_acc_v3*(*ej2).f_acc_v3)/factor_1;
			 			 (*ej2).N_loop_acc_v3++;
			 			 (*ej2).f_cresta_v3=(*ej2).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej2).f_actual_v3=(*ej2).f_max_v3;

				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej2).delta_v3*(*ej2).f_actual_v3+(*ej2).error_v3>factor_1 && (*ej2).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej2).step_pin;
			(*ej2).error_v3=(*ej2).delta_v3*(*ej2).f_actual_v3+(*ej2).error_v3-factor_1;
			if((*ej2).error_v3>factor_1) (*ej2).error_v3=0;
			else{
				(*ej2).null_operation_v3++;
			}
			(*ej2).delta_v3=0;
		   	(*ej2).distancia_v3--;
		}else{
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej2).step_pin;
		}
		(*ej2).delta_v3++;
		GPIOB -> ODR &= ~(*ej2).step_pin;

	}


	GPIOB -> ODR &= ~(*ej0).step_pin;
	GPIOB -> ODR &= ~(*ej1).step_pin;
	GPIOB -> ODR &= ~(*ej2).step_pin;
	(*ej0).steps_actual=(*ej0).steps_goto;
	(*ej1).steps_actual=(*ej1).steps_goto;
	(*ej2).steps_actual=(*ej2).steps_goto;
}

/* Description:
 *			generates 1-axis motion
 * Parameters:
 *			struct eje_str *ej0: axis pointer 1
 *			struct eje_str *ej1: axis pointer 2
 *			struct eje_str *ej2: axis pointer 3
 *			struct eje_str *ej3: axis pointer 4
 * Return:
 *
 * Notes:
 *
 */
void run_4_motor(struct eje_str *ej0, struct eje_str *ej1, struct eje_str *ej2, struct eje_str *ej3){
	uint32_t factor_1=F_BASE_Hz/4;

	while ((*ej0).distancia_v3+(*ej1).distancia_v3+(*ej2).distancia_v3+(*ej3).distancia_v3>0){
		if(GPIOA -> IDR & E_Stop_Pin) e_stop_routine();

		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset

		  if((*ej0).distancia_v3<(*ej0).step_3_v3){			//CASO 3
			  (*ej0).f_actual_v3=(*ej0).f_cresta_v3-((*ej0).N_loop_desacc_v3*(*ej0).f_acc_v3)/factor_1;
			  if((*ej0).f_actual_v3<=(*ej0).f_min_v3){		//CASO 4
				  (*ej0).f_actual_v3=(*ej0).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej0).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej0).distancia_v3>=(*ej0).step_1_v3){				//CASO 1
			 			 (*ej0).f_actual_v3=((*ej0).N_loop_acc_v3*(*ej0).f_acc_v3)/factor_1;
			 			 (*ej0).N_loop_acc_v3++;
			 			 (*ej0).f_cresta_v3=(*ej0).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej0).f_actual_v3=(*ej0).f_max_v3;

				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3>factor_1 && (*ej0).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej0).step_pin;
			(*ej0).error_v3=(*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3-factor_1;
			if((*ej0).error_v3>factor_1) (*ej0).error_v3=0;
			else{
				(*ej0).null_operation_v3++;
			}
			(*ej0).delta_v3=0;
		   	(*ej0).distancia_v3--;
		}else{
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej0).step_pin;
		}
		(*ej0).delta_v3++;
		GPIOB -> ODR &= ~(*ej0).step_pin;

		  if((*ej1).distancia_v3<(*ej1).step_3_v3){			//CASO 3
			  (*ej1).f_actual_v3=(*ej1).f_cresta_v3-((*ej1).N_loop_desacc_v3*(*ej1).f_acc_v3)/factor_1;
			  if((*ej1).f_actual_v3<=(*ej1).f_min_v3){		//CASO 4
				  (*ej1).f_actual_v3=(*ej1).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej1).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej1).distancia_v3>=(*ej1).step_1_v3){				//CASO 1
			 			 (*ej1).f_actual_v3=((*ej1).N_loop_acc_v3*(*ej1).f_acc_v3)/factor_1;
			 			 (*ej1).N_loop_acc_v3++;
			 			 (*ej1).f_cresta_v3=(*ej1).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej1).f_actual_v3=(*ej1).f_max_v3;

				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3>factor_1 && (*ej1).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej1).step_pin;
			(*ej1).error_v3=(*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3-factor_1;
			if((*ej1).error_v3>factor_1) (*ej1).error_v3=0;
			else{
				(*ej1).null_operation_v3++;
			}
			(*ej1).delta_v3=0;
		   	(*ej1).distancia_v3--;
		}else{
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej1).step_pin;
		}
		(*ej1).delta_v3++;
		GPIOB -> ODR &= ~(*ej1).step_pin;

		  if((*ej2).distancia_v3<(*ej2).step_3_v3){			//CASO 3
			  (*ej2).f_actual_v3=(*ej2).f_cresta_v3-((*ej2).N_loop_desacc_v3*(*ej2).f_acc_v3)/factor_1;
			  if((*ej2).f_actual_v3<=(*ej2).f_min_v3){		//CASO 4
				  (*ej2).f_actual_v3=(*ej2).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej2).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej2).distancia_v3>=(*ej2).step_1_v3){				//CASO 1
			 			 (*ej2).f_actual_v3=((*ej2).N_loop_acc_v3*(*ej2).f_acc_v3)/factor_1;
			 			 (*ej2).N_loop_acc_v3++;
			 			 (*ej2).f_cresta_v3=(*ej2).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej2).f_actual_v3=(*ej2).f_max_v3;

				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej2).delta_v3*(*ej2).f_actual_v3+(*ej2).error_v3>factor_1 && (*ej2).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej2).step_pin;
			(*ej2).error_v3=(*ej2).delta_v3*(*ej2).f_actual_v3+(*ej2).error_v3-factor_1;
			if((*ej2).error_v3>factor_1) (*ej2).error_v3=0;
			else{
				(*ej2).null_operation_v3++;
			}
			(*ej2).delta_v3=0;
		   	(*ej2).distancia_v3--;
		}else{
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej2).step_pin;
		}
		(*ej2).delta_v3++;
		GPIOB -> ODR &= ~(*ej2).step_pin;

		  if((*ej3).distancia_v3<(*ej3).step_3_v3){			//CASO 3
			  (*ej3).f_actual_v3=(*ej3).f_cresta_v3-((*ej3).N_loop_desacc_v3*(*ej3).f_acc_v3)/factor_1;
			  if((*ej3).f_actual_v3<=(*ej3).f_min_v3){		//CASO 4
				  (*ej3).f_actual_v3=(*ej3).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej3).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej3).distancia_v3>=(*ej3).step_1_v3){				//CASO 1
			 			 (*ej3).f_actual_v3=((*ej3).N_loop_acc_v3*(*ej3).f_acc_v3)/factor_1;
			 			 (*ej3).N_loop_acc_v3++;
			 			 (*ej3).f_cresta_v3=(*ej3).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej3).f_actual_v3=(*ej3).f_max_v3;

				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej3).delta_v3*(*ej3).f_actual_v3+(*ej3).error_v3>factor_1 && (*ej3).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej3).step_pin;
			(*ej3).error_v3=(*ej3).delta_v3*(*ej3).f_actual_v3+(*ej3).error_v3-factor_1;
			if((*ej3).error_v3>factor_1) (*ej3).error_v3=0;
			else{
				(*ej3).null_operation_v3++;
			}
			(*ej3).delta_v3=0;
		   	(*ej3).distancia_v3--;
		}else{
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej3).step_pin;
		}
		(*ej3).delta_v3++;
		GPIOB -> ODR &= ~(*ej3).step_pin;

	}


	GPIOB -> ODR &= ~(*ej0).step_pin;
	GPIOB -> ODR &= ~(*ej1).step_pin;
	GPIOB -> ODR &= ~(*ej2).step_pin;
	GPIOB -> ODR &= ~(*ej3).step_pin;
	(*ej0).steps_actual=(*ej0).steps_goto;
	(*ej1).steps_actual=(*ej1).steps_goto;
	(*ej2).steps_actual=(*ej2).steps_goto;
	(*ej3).steps_actual=(*ej3).steps_goto;
}

/* Description:
 *			generates 5-axis motion
 * Parameters:
 *			struct eje_str *ej0: axis pointer 1
 *			struct eje_str *ej1: axis pointer 2
 *			struct eje_str *ej2: axis pointer 3
 *			struct eje_str *ej3: axis pointer 4
 *			struct eje_str *ej4: axis pointer 5
 * Return:
 *
 * Notes:
 *
 */
void run_5_motor(struct eje_str *ej0, struct eje_str *ej1, struct eje_str *ej2, struct eje_str *ej3, struct eje_str *ej4){
	uint32_t factor_1=F_BASE_Hz/5;

	while ((*ej0).distancia_v3+(*ej1).distancia_v3+(*ej2).distancia_v3+(*ej3).distancia_v3+(*ej4).distancia_v3>0){
		if(GPIOA -> IDR & E_Stop_Pin) e_stop_routine();

		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset

		  if((*ej0).distancia_v3<(*ej0).step_3_v3){			//CASO 3
			  (*ej0).f_actual_v3=(*ej0).f_cresta_v3-((*ej0).N_loop_desacc_v3*(*ej0).f_acc_v3)/factor_1;
			  if((*ej0).f_actual_v3<=(*ej0).f_min_v3){		//CASO 4
				  (*ej0).f_actual_v3=(*ej0).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej0).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej0).distancia_v3>=(*ej0).step_1_v3){				//CASO 1
			 			 (*ej0).f_actual_v3=((*ej0).N_loop_acc_v3*(*ej0).f_acc_v3)/factor_1;
			 			 (*ej0).N_loop_acc_v3++;
			 			 (*ej0).f_cresta_v3=(*ej0).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej0).f_actual_v3=(*ej0).f_max_v3;

				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
				(*ej0).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3>factor_1 && (*ej0).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej0).step_pin;
			(*ej0).error_v3=(*ej0).delta_v3*(*ej0).f_actual_v3+(*ej0).error_v3-factor_1;
			if((*ej0).error_v3>factor_1) (*ej0).error_v3=0;
			else{
				(*ej0).null_operation_v3++;
			}
			(*ej0).delta_v3=0;
		   	(*ej0).distancia_v3--;
		}else{
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			(*ej0).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej0).step_pin;
		}
		(*ej0).delta_v3++;
		GPIOB -> ODR &= ~(*ej0).step_pin;

		  if((*ej1).distancia_v3<(*ej1).step_3_v3){			//CASO 3
			  (*ej1).f_actual_v3=(*ej1).f_cresta_v3-((*ej1).N_loop_desacc_v3*(*ej1).f_acc_v3)/factor_1;
			  if((*ej1).f_actual_v3<=(*ej1).f_min_v3){		//CASO 4
				  (*ej1).f_actual_v3=(*ej1).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej1).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej1).distancia_v3>=(*ej1).step_1_v3){				//CASO 1
			 			 (*ej1).f_actual_v3=((*ej1).N_loop_acc_v3*(*ej1).f_acc_v3)/factor_1;
			 			 (*ej1).N_loop_acc_v3++;
			 			 (*ej1).f_cresta_v3=(*ej1).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej1).f_actual_v3=(*ej1).f_max_v3;

				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
				(*ej1).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3>factor_1 && (*ej1).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej1).step_pin;
			(*ej1).error_v3=(*ej1).delta_v3*(*ej1).f_actual_v3+(*ej1).error_v3-factor_1;
			if((*ej1).error_v3>factor_1) (*ej1).error_v3=0;
			else{
				(*ej1).null_operation_v3++;
			}
			(*ej1).delta_v3=0;
		   	(*ej1).distancia_v3--;
		}else{
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			(*ej1).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej1).step_pin;
		}
		(*ej1).delta_v3++;
		GPIOB -> ODR &= ~(*ej1).step_pin;

		  if((*ej2).distancia_v3<(*ej2).step_3_v3){			//CASO 3
			  (*ej2).f_actual_v3=(*ej2).f_cresta_v3-((*ej2).N_loop_desacc_v3*(*ej2).f_acc_v3)/factor_1;
			  if((*ej2).f_actual_v3<=(*ej2).f_min_v3){		//CASO 4
				  (*ej2).f_actual_v3=(*ej2).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej2).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej2).distancia_v3>=(*ej2).step_1_v3){				//CASO 1
			 			 (*ej2).f_actual_v3=((*ej2).N_loop_acc_v3*(*ej2).f_acc_v3)/factor_1;
			 			 (*ej2).N_loop_acc_v3++;
			 			 (*ej2).f_cresta_v3=(*ej2).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej2).f_actual_v3=(*ej2).f_max_v3;

				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
				(*ej2).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej2).delta_v3*(*ej2).f_actual_v3+(*ej2).error_v3>factor_1 && (*ej2).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej2).step_pin;
			(*ej2).error_v3=(*ej2).delta_v3*(*ej2).f_actual_v3+(*ej2).error_v3-factor_1;
			if((*ej2).error_v3>factor_1) (*ej2).error_v3=0;
			else{
				(*ej2).null_operation_v3++;
			}
			(*ej2).delta_v3=0;
		   	(*ej2).distancia_v3--;
		}else{
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			(*ej2).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej2).step_pin;
		}
		(*ej2).delta_v3++;
		GPIOB -> ODR &= ~(*ej2).step_pin;

		  if((*ej3).distancia_v3<(*ej3).step_3_v3){			//CASO 3
			  (*ej3).f_actual_v3=(*ej3).f_cresta_v3-((*ej3).N_loop_desacc_v3*(*ej3).f_acc_v3)/factor_1;
			  if((*ej3).f_actual_v3<=(*ej3).f_min_v3){		//CASO 4
				  (*ej3).f_actual_v3=(*ej3).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej3).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej3).distancia_v3>=(*ej3).step_1_v3){				//CASO 1
			 			 (*ej3).f_actual_v3=((*ej3).N_loop_acc_v3*(*ej3).f_acc_v3)/factor_1;
			 			 (*ej3).N_loop_acc_v3++;
			 			 (*ej3).f_cresta_v3=(*ej3).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej3).f_actual_v3=(*ej3).f_max_v3;

				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
				(*ej3).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej3).delta_v3*(*ej3).f_actual_v3+(*ej3).error_v3>factor_1 && (*ej3).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej3).step_pin;
			(*ej3).error_v3=(*ej3).delta_v3*(*ej3).f_actual_v3+(*ej3).error_v3-factor_1;
			if((*ej3).error_v3>factor_1) (*ej3).error_v3=0;
			else{
				(*ej3).null_operation_v3++;
			}
			(*ej3).delta_v3=0;
		   	(*ej3).distancia_v3--;
		}else{
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			(*ej3).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej3).step_pin;
		}
		(*ej3).delta_v3++;
		GPIOB -> ODR &= ~(*ej3).step_pin;

		  if((*ej4).distancia_v3<(*ej4).step_3_v3){			//CASO 3
			  (*ej4).f_actual_v3=(*ej4).f_cresta_v3-((*ej4).N_loop_desacc_v3*(*ej4).f_acc_v3)/factor_1;
			  if((*ej4).f_actual_v3<=(*ej4).f_min_v3){		//CASO 4
				  (*ej4).f_actual_v3=(*ej4).f_min_v3;
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
				   	asm("NOP");
			  }else{
				  (*ej4).N_loop_desacc_v3++;
			  }
		  }else{
			 if((*ej4).distancia_v3>=(*ej4).step_1_v3){				//CASO 1
			 			 (*ej4).f_actual_v3=((*ej4).N_loop_acc_v3*(*ej4).f_acc_v3)/factor_1;
			 			 (*ej4).N_loop_acc_v3++;
			 			 (*ej4).f_cresta_v3=(*ej4).f_actual_v3;
			 }else{												//CASO 2
		 		(*ej4).f_actual_v3=(*ej4).f_max_v3;

				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
				(*ej4).null_operation_v3++;
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
		 	 }
		 }

		if ((*ej4).delta_v3*(*ej4).f_actual_v3+(*ej4).error_v3>factor_1 && (*ej4).distancia_v3!=0){
		   	GPIOB -> ODR |= (*ej4).step_pin;
			(*ej4).error_v3=(*ej4).delta_v3*(*ej4).f_actual_v3+(*ej4).error_v3-factor_1;
			if((*ej4).error_v3>factor_1) (*ej4).error_v3=0;
			else{
				(*ej4).null_operation_v3++;
			}
			(*ej4).delta_v3=0;
		   	(*ej4).distancia_v3--;
		}else{
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			(*ej4).null_operation_v3++;
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
		   	GPIOB -> ODR &= ~(*ej4).step_pin;
		}
		(*ej4).delta_v3++;
		GPIOB -> ODR &= ~(*ej4).step_pin;

	}


	GPIOB -> ODR &= ~(*ej0).step_pin;
	GPIOB -> ODR &= ~(*ej1).step_pin;
	GPIOB -> ODR &= ~(*ej2).step_pin;
	GPIOB -> ODR &= ~(*ej3).step_pin;
	GPIOB -> ODR &= ~(*ej4).step_pin;
	(*ej0).steps_actual=(*ej0).steps_goto;
	(*ej1).steps_actual=(*ej1).steps_goto;
	(*ej2).steps_actual=(*ej2).steps_goto;
	(*ej3).steps_actual=(*ej3).steps_goto;
	(*ej4).steps_actual=(*ej4).steps_goto;
}

/*** end of file ***/
