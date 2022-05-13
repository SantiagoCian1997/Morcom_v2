
#include "home_m.h"

extern void run_motores(double,double);
extern struct eje_str X,Y,Z,B,C;
void home_selec_eje(uint8_t);

void home_inicio_sec(uint8_t home_inicio){
	  if(home_inicio==0){
			X.step_mm_goto=0;
			Y.step_mm_goto=0;
			Z.step_mm_goto=0;
			B.step_mm_goto=0;
			C.step_mm_goto=0;
			X.step_mm_actual=X.step_mm_goto;
			Y.step_mm_actual=Y.step_mm_goto;
			Z.step_mm_actual=Z.step_mm_goto;
			B.step_mm_actual=B.step_mm_goto;
			C.step_mm_actual=C.step_mm_goto;

	  }
	  if(home_inicio==1){
			home_secuencia();
			X.step_mm_goto=X.home_ubic_mm*X.step_mm;
			Y.step_mm_goto=Y.home_ubic_mm*Y.step_mm;
			Z.step_mm_goto=Z.home_ubic_mm*Z.step_mm;
			B.step_mm_goto=B.home_ubic_mm*B.step_mm;
			C.step_mm_goto=C.home_ubic_mm*C.step_mm;
			X.step_mm_actual=X.step_mm_goto;
			Y.step_mm_actual=Y.step_mm_goto;
			Z.step_mm_actual=Z.step_mm_goto;
			B.step_mm_actual=B.step_mm_goto;
			C.step_mm_actual=C.step_mm_goto;
	  }
	  if(home_inicio==2){
			X.step_mm_goto=X.home_ubic_mm*X.step_mm;
			Y.step_mm_goto=Y.home_ubic_mm*Y.step_mm;
			Z.step_mm_goto=Z.home_ubic_mm*Z.step_mm;
			B.step_mm_goto=B.home_ubic_mm*B.step_mm;
			C.step_mm_goto=C.home_ubic_mm*C.step_mm;
			X.step_mm_actual=X.step_mm_goto;
			Y.step_mm_actual=Y.step_mm_goto;
			Z.step_mm_actual=Z.step_mm_goto;
			B.step_mm_actual=B.step_mm_goto;
			C.step_mm_actual=C.step_mm_goto;
	  }

}

void home_secuencia(){
	uint8_t home_prioridad[5]={X.home_prioridad,Y.home_prioridad,Z.home_prioridad,B.home_prioridad,C.home_prioridad};
	uint8_t home_secuencia_arr[5]={0,1,2,3,4};
	uint8_t aux_ret;
	for(uint8_t i=0;i<5;i++){
	    for(uint8_t j=0;j<5-1;j++){
	      if(home_prioridad[j]>home_prioridad[j+1]) {
	    	  aux_ret=home_prioridad[j];
	    	  home_prioridad[j]=home_prioridad[j+1];
	    	  home_prioridad[j+1]=aux_ret;
	    	  aux_ret=home_secuencia_arr[j];
	    	  home_secuencia_arr[j]=home_secuencia_arr[j+1];
	    	  home_secuencia_arr[j+1]=aux_ret;
	      }
	    }
	}

	for(uint8_t i=0;i<5;i++){
		home_selec_eje(home_secuencia_arr[i]);
	}
	X.step_mm_goto=X.home_ubic_mm*X.step_mm;
	Y.step_mm_goto=Y.home_ubic_mm*Y.step_mm;
	Z.step_mm_goto=Z.home_ubic_mm*Z.step_mm;
	B.step_mm_goto=B.home_ubic_mm*B.step_mm;
	C.step_mm_goto=C.home_ubic_mm*C.step_mm;
	X.step_mm_actual=X.step_mm_goto;
	Y.step_mm_actual=Y.step_mm_goto;
	Z.step_mm_actual=Z.step_mm_goto;
	B.step_mm_actual=B.step_mm_goto;
	C.step_mm_actual=C.step_mm_goto;
}
//GPIOB -> ODR &= ~pin_step;//reset//
//GPIOB -> ODR |= pin_step;//set
void homing_eje(struct eje_str *eje,uint16_t pin_step,uint16_t pin_dir,uint16_t pin_switch){
	uint32_t f_busqueda=((*eje).v_max*(*eje).home_porcentaje_vel*(*eje).step_mm)/100;
	uint32_t f_busqeuda_lenta=f_busqueda/5;
	uint32_t f_loop=50000;
	uint32_t delta=0;

	//printf("f_busqeuda:%i,pin_dir:%i,pin_step:%i\n",f_busqueda,pin_dir,pin_step);
	if(f_busqeuda_lenta==0) f_busqeuda_lenta=1;
	if((*eje).home_act){

		HAL_GPIO_WritePin(GPIOB, (*eje).dir_pin, 1-(*eje).home_dir);
		if((*eje).invertir_direccion) HAL_GPIO_TogglePin(GPIOB, (*eje).dir_pin);

		while(!HAL_GPIO_ReadPin(GPIOA, (*eje).home_pin)){//mover en la direccion hasta pulsar el fin de carrera
			if (delta*f_busqueda>f_loop){
				GPIOB -> ODR |= (*eje).step_pin;//set
				delta=0;
			}
			delta++;
			for(uint32_t i=0;i<100;i++) asm("NOP");
			GPIOB -> ODR &= ~(*eje).step_pin;//reset
		}

		for(uint32_t h=0;h<1500000;h++){
		   	asm("NOP");
		   	asm("NOP");
		}

		delta=0;
		HAL_GPIO_TogglePin(GPIOB, (*eje).dir_pin);
		while(HAL_GPIO_ReadPin(GPIOA, (*eje).home_pin)){//mover en direccion opuesta hasta despulsar fin de carrera
					if (delta*f_busqeuda_lenta>f_loop){
						GPIOB -> ODR |= pin_step;//set
						delta=0;
					}
					delta++;
					for(uint32_t i=0;i<100;i++) asm("NOP");
					GPIOB -> ODR &= ~pin_step;//reset
		}

		for(uint32_t h=0;h<1500000;h++){
		   	asm("NOP");
		   	asm("NOP");
		}

		(*eje).step_mm_goto=(*eje).home_mm_retroceso*(*eje).step_mm;//mover los milimetros seteados en el home
		if(!(*eje).home_dir) (*eje).step_mm_goto=-(*eje).step_mm_goto;
		(*eje).step_mm_actual=0;
		run_motores(0,0);//mover en G0 F0

		/*while(steps_retroceso!=0){
			if (delta*f_busqeuda_lenta>f_loop){
				GPIOB -> ODR |= pin_step;//set
				delta=0;
				steps_retroceso--;
			}
			delta++;
			for(uint32_t i=0;i<100;i++) asm("NOP");
			GPIOB -> ODR &= ~pin_step;//reset
		}*/

		/*
		delta=0;
		HAL_GPIO_TogglePin(GPIOB, pin_dir);
		while(HAL_GPIO_ReadPin(GPIOA, pin_switch)!=eje.home_sw_nromal_0_1){
			if (delta*f_busqeuda_lenta>f_loop){
				GPIOB -> ODR |= pin_step;//set
				delta=0;
			}
			delta++;
			for(uint32_t i=0;i<188;i++) asm("NOP");
			GPIOB -> ODR &= ~pin_step;//reset

			//  GPIOB -> ODR |= Dir_B_Pin;//set
			//  GPIOB -> ODR &= ~Dir_B_Pin;//reset
		}*/
	}
}

void home_selec_eje(uint8_t i){
	printf("buscnado:%i\n",i);
	switch (i){
	case 0:{homing_eje(&X,Step_X_Pin,Dir_X_Pin,Home_X_Pin);
	}break;
	case 1:{homing_eje(&Y,Step_Y_Pin,Dir_Y_Pin,Home_Y_Pin);
	}break;
	case 2:{homing_eje(&Z,Step_Z_Pin,Dir_Z_Pin,Home_Z_Pin);
	}break;
	case 3:{homing_eje(&B,Step_B_Pin,Dir_B_Pin,Home_B_Pin);
	}break;
	case 4:{homing_eje(&C,Step_C_Pin,Dir_C_Pin,Home_C_Pin);
	}break;
	default:{
	}break;
	}
}
