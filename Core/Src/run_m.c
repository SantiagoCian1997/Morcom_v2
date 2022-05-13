
#include "run_m.h"
#include "main.h"

/*
typedef struct __eje_struct{//parametros fijos, levantados de la flash en el arranque o en el regrabado

	double mm_actual;
	double mm_goto;
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



}eje_struct;*/
extern struct eje_str X,Y,Z,B,C;
extern uint8_t parada_de_emergencia;
typedef struct __eje_run_struct{//parametros que modifica y utiliza la funcion run_motor para correr

	int32_t step_actual;
	int32_t step_goto;
	int32_t step_goto_05;
	int32_t error;
	int64_t acc_Hz;
	int64_t f_max;
	int64_t f_actual;
	int64_t f_cresta;
	int64_t f_min;
	uint64_t loop_aceleracion;
	uint64_t loop_desaceleracion;
	uint32_t delta;
}eje_run_struct;
eje_run_struct Xr,Yr,Zr,Br,Cr;
//float intervalo_loop_s=0.00001;//tiempo de cada loop en segundos
//float intervalo_loop_s=0.000004917;//tiempo de cada loop en segundos
//float intervalo_loop_s=0.000014751;//tiempo de cada loop en segundos
//float intervalo_loop_s=0.000069000;//tiempo de cada loop en segundos	5 motores, cristal 24MHz
//float intervalo_loop_s=0.000061875;//tiempo de cada loop en segundos	5 motores, cristal 32MHz
//float intervalo_loop_s=0.000042500;//tiempo de cada loop en segundos	5 motores, cristal 56MHz
//float intervalo_loop_s=0.0000411822;//tiempo de cada loop en segundos	5 motores, cristal 56MHz retocado
//float   intervalo_loop_s=0.00003718875;//tiempo de cada loop en segundos	5 motores, cristal 72MHz

uint32_t factor=1;//es la inverza del intervalo
uint32_t factor_5=1;//es la inverza del intervalo


uint32_t factor_3=1;//es la inverza del intervalo
uint32_t factor_2=1;//es la inverza del intervalo

void eje_iniciacion(eje_run_struct *eje_r, struct eje_str eje){
	if(eje.step_mm_actual<eje.step_mm_goto) {//movimiento
		GPIOB -> ODR |= eje.dir_pin;//set//set estado pin direccion
		//eje_r.step_goto=(uint32_t)(((double)(eje.mm_goto-eje.mm_actual))*eje.step_mm);
		(*eje_r).step_goto=eje.step_mm_goto;
		(*eje_r).step_goto-=eje.step_mm_actual;
	}
	if(eje.step_mm_actual>eje.step_mm_goto) {
		GPIOB -> ODR &= ~eje.dir_pin;//reset//set estado pin direccion
		//eje_r.step_goto=(uint32_t)(((double)(eje.mm_actual-eje.mm_goto))*eje.step_mm);
		(*eje_r).step_goto=eje.step_mm_actual;
		(*eje_r).step_goto-=eje.step_mm_goto;
	}
	if(eje.step_mm_actual==eje.step_mm_goto)  (*eje_r).step_goto=0;
	if(eje.invertir_direccion) HAL_GPIO_TogglePin(GPIOB, eje.dir_pin);

	(*eje_r).f_max=eje.v_G1*eje.step_mm;//uso v_G1 porque se setea aca el valor de F llegado el caso //falta interpolacion
	(*eje_r).f_min=((*eje_r).f_max*eje.porcentaje_v_min)/100;			//falta interpolacion
	(*eje_r).acc_Hz=(eje.v_G1*eje.acc*eje.step_mm)/eje.v_max;							//falta interpolacion

	if((*eje_r).acc_Hz==0) (*eje_r).acc_Hz=1;
	if((*eje_r).f_max==0) (*eje_r).f_max=1;
	(*eje_r).step_actual=0;
	(*eje_r).step_goto_05=(*eje_r).step_goto/2;
	(*eje_r).f_actual=0;
	(*eje_r).loop_desaceleracion=0;
	(*eje_r).loop_aceleracion=0;
	(*eje_r).error=0;
	(*eje_r).delta=0;

}
uint16_t E_stop_N_error=0;//numero de error 0:nada, 1:Xpositivo, 2:Xnegativo, 3:Ypositivo, 4:Ynegativo...
void despulsar_eje(struct eje_str eje){
	if(eje.home_act==1){
	HAL_GPIO_TogglePin(GPIOB, eje.dir_pin);//cambio la direccion
	//uint32_t N=23988;//22578 constante del loop
	uint32_t N=30000;//modificada porque era muy rapida (estaba bien igual)
	double AUX=10000/(eje.v_max*eje.step_mm*eje.home_porcentaje_vel);
	N=N*AUX;
	N-=33;//33=constante del loop

	for(uint32_t i=0;i<eje.home_mm_retroceso*eje.step_mm*2;i++){
		if(!HAL_GPIO_ReadPin(GPIOA, eje.home_pin)) i=eje.home_mm_retroceso*eje.step_mm*2;//salgo del for ni bien despulso, en caso de mantener pulsado solo retrocede lo indicado en el home
		HAL_GPIO_TogglePin(GPIOB, eje.step_pin);
		for(uint32_t h=0;h<N;h++){
		   	asm("NOP");
		   	asm("NOP");
		}
	}
	}
}
void E_stop_rutina(){
	printf("E_stop!\n");
	parada_de_emergencia=1;
	for(uint32_t h=0;h<1500000;h++){
	   	asm("NOP");
	   	asm("NOP");
	}
	E_stop_N_error=0;
	if(HAL_GPIO_ReadPin(GPIOA, X.home_pin)){
		printf("desp_X\n");
		if(X.step_mm_actual<X.step_mm_goto) 		E_stop_N_error=1;
		else 										E_stop_N_error=2;
		despulsar_eje(X);
	}
	if(HAL_GPIO_ReadPin(GPIOA, Y.home_pin)){
		printf("desp_Y\n");
		if(Y.step_mm_actual<Y.step_mm_goto) 		E_stop_N_error=3;
		else 										E_stop_N_error=4;
		despulsar_eje(Y);
	}
	if(HAL_GPIO_ReadPin(GPIOA, Z.home_pin)){
		printf("desp_Z\n");
		if(Z.step_mm_actual<Z.step_mm_goto) 		E_stop_N_error=5;
		else 										E_stop_N_error=6;
		despulsar_eje(Z);
	}
	if(HAL_GPIO_ReadPin(GPIOA, B.home_pin)){
		printf("desp_B\n");
		if(B.step_mm_actual<B.step_mm_goto) 		E_stop_N_error=7;
		else 										E_stop_N_error=8;
		despulsar_eje(B);
	}
	if(HAL_GPIO_ReadPin(GPIOA, C.home_pin)){
		printf("desp_C\n");
		if(C.step_mm_actual<C.step_mm_goto) 		E_stop_N_error=8;
		else 										E_stop_N_error=9;
		despulsar_eje(C);
	}
	Xr.step_actual=Xr.step_goto;//para salir del loop de run
	Yr.step_actual=Yr.step_goto;
	Zr.step_actual=Zr.step_goto;
	Br.step_actual=Br.step_goto;
	Cr.step_actual=Cr.step_goto;
}
void run_motores(double G_value,double F_value){

	//factor=	1/intervalo_loop_s;

	//factor_5=26890;//5 motores, cristal 72MHz
	//factor=127250;//f de loop de un solo motor
	factor=105130;//f de loop de un solo motor

	factor_3=factor/3;
	factor_2=factor/2;

	factor=25360;

	if(F_value==0)F_value=1;
	if(G_value==0) {
		X.v_G1=X.v_max;
		Y.v_G1=Y.v_max;
		Z.v_G1=Z.v_max;
		B.v_G1=B.v_max;
		C.v_G1=C.v_max;
	}
	if(G_value==1) {
		if(F_value>X.v_max) X.v_G1=X.v_max;
		else X.v_G1=F_value;
		if(F_value>Y.v_max) Y.v_G1=Y.v_max;
		else Y.v_G1=F_value;
		if(F_value>Z.v_max) Z.v_G1=Z.v_max;
		else Z.v_G1=F_value;
		if(F_value>B.v_max) B.v_G1=B.v_max;
		else B.v_G1=F_value;
		if(F_value>C.v_max) C.v_G1=C.v_max;
		else C.v_G1=F_value;
	}
	eje_iniciacion(&Xr,X);
	eje_iniciacion(&Yr,Y);
	eje_iniciacion(&Zr,Z);
	eje_iniciacion(&Br,B);
	eje_iniciacion(&Cr,C);




	int64_t suma=1;
	while (suma!=0){
		if(GPIOA -> IDR & E_Stop_Pin) E_stop_rutina();

		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset
		  //GPIOB -> ODR |= Step_C_Pin;//set
		  //GPIOB -> ODR &= ~Step_C_Pin;//reset

		 if (Xr.step_actual<=Xr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Xr.f_actual=(Xr.loop_aceleracion*Xr.acc_Hz)/factor_3;//% delta_t.aceleracion.intervalo=f=a.t
		   	Xr.f_cresta=Xr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Xr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Xr.step_actual>Xr.step_goto_05){//else del if anterior
		   	Xr.f_actual=Xr.f_cresta-(Xr.loop_desaceleracion*Xr.acc_Hz)/factor_3;
		   	Xr.loop_desaceleracion++;
		   	if (Xr.f_actual<Xr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Xr.f_actual=Xr.f_min;
		   	}
		}
		if (Xr.f_actual>=Xr.f_max){
		   	Xr.f_actual=Xr.f_max;
		}
		if (Xr.f_actual<Xr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_X_Pin;//reset




		if (Xr.delta*Xr.f_actual+Xr.error>factor_3&&Xr.step_actual!=Xr.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Xr.error=Xr.delta*Xr.f_actual+Xr.error-factor_3;
			if(Xr.error>factor_3) Xr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
			Xr.delta=0;
		   	Xr.step_actual++;
		   	GPIOB -> ODR |= Step_X_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Xr.f_actual,Xr.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_X_Pin;//reset//para mantener la cantidad de sentencias
		}
		Xr.delta++;
		 if (Yr.step_actual<=Yr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Yr.f_actual=(Yr.loop_aceleracion*Yr.acc_Hz)/factor_3;//% delta_t.aceleracion.intervalo=f=a.t
		   	Yr.f_cresta=Yr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Yr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Yr.step_actual>Yr.step_goto_05){//else del if anterior
		   	Yr.f_actual=Yr.f_cresta-(Yr.loop_desaceleracion*Yr.acc_Hz)/factor_3;
		   	Yr.loop_desaceleracion++;
		   	if (Yr.f_actual<Yr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Yr.f_actual=Yr.f_min;
		   	}
		}
		if (Yr.f_actual>=Yr.f_max){
		   	Yr.f_actual=Yr.f_max;
		}
		if (Yr.f_actual<Yr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_Y_Pin;//reset
		if (Yr.delta*Yr.f_actual+Yr.error>factor_3&&Yr.step_actual!=Yr.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Yr.error=Yr.delta*Yr.f_actual+Yr.error-factor_3;
			if(Yr.error>factor_3) Yr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Yr.delta=0;
		   	Yr.step_actual++;
		   	GPIOB -> ODR |= Step_Y_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Yr.f_actual,Yr.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_Y_Pin;//reset//para mantener la cantidad de sentencias
		}
		Yr.delta++;

		 if (Cr.step_actual<=Cr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Cr.f_actual=(Cr.loop_aceleracion*Cr.acc_Hz)/factor_3;//% delta_t.aceleracion.intervalo=f=a.t
		   	Cr.f_cresta=Cr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Cr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Cr.step_actual>Cr.step_goto_05){//else del if anterior
		   	Cr.f_actual=Cr.f_cresta-(Cr.loop_desaceleracion*Cr.acc_Hz)/factor_3;
		   	Cr.loop_desaceleracion++;
		   	if (Cr.f_actual<Cr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Cr.f_actual=Cr.f_min;
		   	}
		}
		if (Cr.f_actual>=Cr.f_max){
		   	Cr.f_actual=Cr.f_max;
		}
		if (Cr.f_actual<Cr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_C_Pin;//reset
		if (Cr.delta*Cr.f_actual+Cr.error>factor_3&&Cr.step_actual!=Cr.step_goto){/////////PROCAR CON SUMAR EL ERROR
			Cr.error=Cr.delta*Cr.f_actual+Cr.error-factor_3;
			if(Cr.error>factor_3) Cr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Cr.delta=0;
		   	Cr.step_actual++;
		   	GPIOB -> ODR |= Step_C_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Cr.f_actual,Cr.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_C_Pin;//reset//para mantener la cantidad de sentencias
		}
		Cr.delta++;
		suma=Xr.step_goto-Xr.step_actual+Yr.step_goto-Yr.step_actual+Cr.step_goto-Cr.step_actual;
	}


	GPIOB -> ODR &= ~Step_X_Pin;//reset
	GPIOB -> ODR &= ~Step_Y_Pin;//reset
	GPIOB -> ODR &= ~Step_C_Pin;//reset

	if(parada_de_emergencia==0){
	suma=1;
	while (suma!=0){
		if(GPIOA -> IDR & E_Stop_Pin) E_stop_rutina();
		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset
		  //GPIOB -> ODR |= Step_C_Pin;//set
		  //GPIOB -> ODR &= ~Step_C_Pin;//reset

		 if (Zr.step_actual<=Zr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Zr.f_actual=(Zr.loop_aceleracion*Zr.acc_Hz)/factor_2;//% delta_t.aceleracion.intervalo=f=a.t
		   	Zr.f_cresta=Zr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Zr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Zr.step_actual>Zr.step_goto_05){//else del if anterior
		   	Zr.f_actual=Zr.f_cresta-(Zr.loop_desaceleracion*Zr.acc_Hz)/factor_2;
		   	Zr.loop_desaceleracion++;
		   	if (Zr.f_actual<Zr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Zr.f_actual=Zr.f_min;
		   	}
		}
		if (Zr.f_actual>=Zr.f_max){
		   	Zr.f_actual=Zr.f_max;
		}
		if (Zr.f_actual<Zr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_Z_Pin;//reset
		if (Zr.delta*Zr.f_actual+Zr.error>factor_2&&Zr.step_actual!=Zr.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Zr.error=Zr.delta*Zr.f_actual+Zr.error-factor_2;
			if(Zr.error>factor_2) Zr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Zr.delta=0;
		   	Zr.step_actual++;
		   	GPIOB -> ODR |= Step_Z_Pin;//set
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_Z_Pin;//reset//para mantener la cantidad de sentencias
		}
		Zr.delta++;
		 if (Br.step_actual<=Br.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Br.f_actual=(Br.loop_aceleracion*Br.acc_Hz)/factor_2;//% delta_t.aceleracion.intervalo=f=a.t
		   	Br.f_cresta=Br.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Br.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Br.step_actual>Br.step_goto_05){//else del if anterior
		   	Br.f_actual=Br.f_cresta-(Br.loop_desaceleracion*Br.acc_Hz)/factor_2;
		   	Br.loop_desaceleracion++;
		   	if (Br.f_actual<Br.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Br.f_actual=Br.f_min;
		   	}
		}
		if (Br.f_actual>=Br.f_max){
		   	Br.f_actual=Br.f_max;
		}
		if (Br.f_actual<Br.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_B_Pin;//reset
		if (Br.delta*Br.f_actual+Br.error>factor_2&&Br.step_actual!=Br.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Br.error=Br.delta*Br.f_actual+Br.error-factor_2;
			if(Br.error>factor_2) Br.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Br.delta=0;
		   	Br.step_actual++;
		   	GPIOB -> ODR |= Step_B_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Br.f_actual,Br.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_B_Pin;//reset//para mantener la cantidad de sentencias
		}
		Br.delta++;
		suma=Zr.step_goto-Zr.step_actual+Br.step_goto-Br.step_actual;
		}
		}

	/*
	int64_t suma=1;
	while (suma!=0){
		  //GPIOB -> ODR |= Dir_B_Pin;//set
		  //GPIOB -> ODR &= ~Dir_B_Pin;//reset
		  GPIOB -> ODR |= Step_C_Pin;//set
		  GPIOB -> ODR &= ~Step_C_Pin;//reset

		 if (Xr.step_actual<=Xr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Xr.f_actual=(Xr.loop_aceleracion*Xr.acc_Hz)/factor;//% delta_t.aceleracion.intervalo=f=a.t
		   	Xr.f_cresta=Xr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Xr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Xr.step_actual>Xr.step_goto_05){//else del if anterior
		   	Xr.f_actual=Xr.f_cresta-(Xr.loop_desaceleracion*Xr.acc_Hz)/factor;
		   	Xr.loop_desaceleracion++;
		   	if (Xr.f_actual<Xr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Xr.f_actual=Xr.f_min;
		   	}
		}
		if (Xr.f_actual>=Xr.f_max){
		   	Xr.f_actual=Xr.f_max;
		}
		if (Xr.f_actual<Xr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_X_Pin;//reset




		if (Xr.delta*Xr.f_actual+Xr.error>factor&&Xr.step_actual!=Xr.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Xr.error=Xr.delta*Xr.f_actual+Xr.error-factor;
			if(Xr.error>factor) Xr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
			Xr.delta=0;
		   	Xr.step_actual++;
		   	GPIOB -> ODR |= Step_X_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Xr.f_actual,Xr.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_X_Pin;//reset//para mantener la cantidad de sentencias
		}
		Xr.delta++;
		 if (Yr.step_actual<=Yr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Yr.f_actual=(Yr.loop_aceleracion*Yr.acc_Hz)/factor;//% delta_t.aceleracion.intervalo=f=a.t
		   	Yr.f_cresta=Yr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Yr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Yr.step_actual>Yr.step_goto_05){//else del if anterior
		   	Yr.f_actual=Yr.f_cresta-(Yr.loop_desaceleracion*Yr.acc_Hz)/factor;
		   	Yr.loop_desaceleracion++;
		   	if (Yr.f_actual<Yr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Yr.f_actual=Yr.f_min;
		   	}
		}
		if (Yr.f_actual>=Yr.f_max){
		   	Yr.f_actual=Yr.f_max;
		}
		if (Yr.f_actual<Yr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_Y_Pin;//reset
		if (Yr.delta*Yr.f_actual+Yr.error>factor&&Yr.step_actual!=Yr.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Yr.error=Yr.delta*Yr.f_actual+Yr.error-factor;
			if(Yr.error>factor) Yr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Yr.delta=0;
		   	Yr.step_actual++;
		   	GPIOB -> ODR |= Step_Y_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Yr.f_actual,Yr.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_Y_Pin;//reset//para mantener la cantidad de sentencias
		}
		Yr.delta++;
		 if (Zr.step_actual<=Zr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Zr.f_actual=(Zr.loop_aceleracion*Zr.acc_Hz)/factor;//% delta_t.aceleracion.intervalo=f=a.t
		   	Zr.f_cresta=Zr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Zr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Zr.step_actual>Zr.step_goto_05){//else del if anterior
		   	Zr.f_actual=Zr.f_cresta-(Zr.loop_desaceleracion*Zr.acc_Hz)/factor;
		   	Zr.loop_desaceleracion++;
		   	if (Zr.f_actual<Zr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Zr.f_actual=Zr.f_min;
		   	}
		}
		if (Zr.f_actual>=Zr.f_max){
		   	Zr.f_actual=Zr.f_max;
		}
		if (Zr.f_actual<Zr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_Z_Pin;//reset
		if (Zr.delta*Zr.f_actual+Zr.error>factor&&Zr.step_actual!=Zr.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Zr.error=Zr.delta*Zr.f_actual+Zr.error-factor;
			if(Zr.error>factor) Zr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Zr.delta=0;
		   	Zr.step_actual++;
		   	GPIOB -> ODR |= Step_Z_Pin;//set
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_Z_Pin;//reset//para mantener la cantidad de sentencias
		}
		Zr.delta++;
		 if (Br.step_actual<=Br.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Br.f_actual=(Br.loop_aceleracion*Br.acc_Hz)/factor;//% delta_t.aceleracion.intervalo=f=a.t
		   	Br.f_cresta=Br.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Br.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Br.step_actual>Br.step_goto_05){//else del if anterior
		   	Br.f_actual=Br.f_cresta-(Br.loop_desaceleracion*Br.acc_Hz)/factor;
		   	Br.loop_desaceleracion++;
		   	if (Br.f_actual<Br.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Br.f_actual=Br.f_min;
		   	}
		}
		if (Br.f_actual>=Br.f_max){
		   	Br.f_actual=Br.f_max;
		}
		if (Br.f_actual<Br.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_B_Pin;//reset
		if (Br.delta*Br.f_actual+Br.error>factor&&Br.step_actual!=Br.step_goto){/////////PROBAR CON SUMAR EL ERROR
			Br.error=Br.delta*Br.f_actual+Br.error-factor;
			if(Br.error>factor) Br.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Br.delta=0;
		   	Br.step_actual++;
		   	GPIOB -> ODR |= Step_B_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Br.f_actual,Br.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_B_Pin;//reset//para mantener la cantidad de sentencias
		}
		Br.delta++;
		 if (Cr.step_actual<=Cr.step_goto_05){// %asi calculo hasta la mitad de la distancia
		   	Cr.f_actual=(Cr.loop_aceleracion*Cr.acc_Hz)/factor;//% delta_t.aceleracion.intervalo=f=a.t
		   	Cr.f_cresta=Cr.f_actual;//%voy guardando el ultimo valor para tener como referencia ocmo valor maximo para el calculo de la desaceleracion
		   	Cr.loop_aceleracion++;
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		if (Cr.step_actual>Cr.step_goto_05){//else del if anterior
		   	Cr.f_actual=Cr.f_cresta-(Cr.loop_desaceleracion*Cr.acc_Hz)/factor;
		   	Cr.loop_desaceleracion++;
		   	if (Cr.f_actual<Cr.f_min){//%esta correccion se hace porque sino no llega mas al final
		       	Cr.f_actual=Cr.f_min;
		   	}
		}
		if (Cr.f_actual>=Cr.f_max){
		   	Cr.f_actual=Cr.f_max;
		}
		if (Cr.f_actual<Cr.f_max){
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		}
		GPIOB -> ODR &= ~Step_C_Pin;//reset
		if (Cr.delta*Cr.f_actual+Cr.error>factor&&Cr.step_actual!=Cr.step_goto){/////////PROCAR CON SUMAR EL ERROR
			Cr.error=Cr.delta*Cr.f_actual+Cr.error-factor;
			if(Cr.error>factor) Cr.error=0;
			else{
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			   	asm("NOP");
			}
		   	Cr.delta=0;
		   	Cr.step_actual++;
		   	GPIOB -> ODR |= Step_C_Pin;//set

		  //printf("f:%i,x:%i,loop:%i\n",Cr.f_actual,Cr.step_actual,loop);
		}else{//else del if anterior
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	asm("NOP");
		   	GPIOB -> ODR &= ~Step_C_Pin;//reset//para mantener la cantidad de sentencias
		}
		Cr.delta++;
		suma=Xr.step_goto-Xr.step_actual+Yr.step_goto-Yr.step_actual+Zr.step_goto-Zr.step_actual+Br.step_goto-Br.step_actual+Cr.step_goto-Cr.step_actual;
	}*/
	GPIOB -> ODR &= ~Step_Z_Pin;//reset
	GPIOB -> ODR &= ~Step_B_Pin;//reset

	X.step_mm_actual=X.step_mm_goto;
	Y.step_mm_actual=Y.step_mm_goto;
	Z.step_mm_actual=Z.step_mm_goto;
	B.step_mm_actual=B.step_mm_goto;
	C.step_mm_actual=C.step_mm_goto;

	/*if(E_stop_N_error!=0){
		if(E_stop_N_error==1) X.step_mm_goto=X.step_mm_actual+X.home_mm_retroceso*X.step_mm;
		if(E_stop_N_error==2) X.step_mm_goto=X.step_mm_actual-X.home_mm_retroceso*X.step_mm;
		run_motores(0, 0);
		E_stop_N_error=0;
	}*/
}

