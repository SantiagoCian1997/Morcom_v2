/*
 * morcom_v2.h
 *
 *  Created on: May, 2022
 *      Author: Cian, Santiago Jose -- santiagocian97@gmail.com
 *              https://github.com/SantiagoCian1997/Morcom_v2
 */

#ifndef MORCOM_V2_H
#define MORCOM_V2_H

#include "stm32f1xx_hal.h"

/*
********************************************************************************
*                   DEFINE CONSTANTS
********************************************************************************
*/
#define N_MOTORS 5

//Indices valores seteo desde la interfaz
#define inicioPagina_flash 0x08018000 //, fin en 0x0801fff0

#define INDEX_N_VERSION_MORCOM				 0x0
//#define index_consulta_splitter_disponibles		0x1
#define INDEX_RETURN_VECTOR_MORCOM			 0x4
//#define index_return_vector_splitter			 0x5	//rango 0-15
#define INDEX_APLICAR_CAMBIOS_MORCOM			 0x6
//#define index_aplicar_cambios_splitter		 0x7	//rango 0-15
#define INDEX_GUARDAR_VECTOR_EN_FLASH_MORCOM	 0x8
//#define index_guardar_vector_en_flash_splitter 0x9	//rango 0-15
#define INDEX_FIN_RETURN_VECTOR_MORCOM		 0xA
//#define index_fin_return_vector_splitter		 0xB	//rango 0-15

#define INDEX_EJE_VEL_MAX 		0x10  //rango 1-2^32
#define INDEX_EJE_ACC 			0x15  //rango 1-2^32
#define INDEX_EJE_STEP_MM		0x1A  //rango (step/mm)*1e6 ->double
#define INDEX_EJE_PERC_VEL 		0x1F  //rango 1-100
#define INDEX_EJE_NEG_DIR		0x24  //rango 1-0

#define INDEX_HOME_ACTIVO					0x100 //rango 0-1
#define INDEX_HOME_DIRECCION				0x105 //rango 0-1
#define INDEX_HOME_PERC_VEL_BUSQUEDA		0x10A //rango 0-100
#define INDEX_HOME_MM_RETROCESO				0x10F //(mm)*1e6
#define INDEX_HOME_UBICACION_FIN_HOME		0x114 //(mm)*1e6 - 0.5*1e6
#define INDEX_HOME_PRIORIDAD_SECUENCIA		0x119 //0-16
#define INDEX_HOME_HACER_AL_INICIO			0x11E //0-2

//M comandos
#define M_VALUE_CONSULTA_SETS 		999   //retorna los valores al usuario
#define M_VALUE_SAVE_SETS 			998   //guarda en la flash

#define M_VALUE_EJE_VEL_MAX 		1000  //rango 0-2^32
#define M_VALUE_EJE_ACC 			1001  //rango 0-2^32
#define M_VALUE_EJE_STEP_MM			1002  //rango double
#define M_VALUE_EJE_PERC_VEL 		1003  //rango 1-100
#define M_VALUE_EJE_NEG_DIR			1004  //rango 0-1

#define M_VALUE_HOME_ACT_EJE		1010  //rango 0-1
#define M_VALUE_HOME_DIR_EJE		1011  //rango 0-1
#define M_VALUE_HOME_PERC_VEL		1012  //rango 0-100
#define M_VALUE_HOME_MM_RETROCESO	1013  //rango 0-2^16
#define M_VALUE_HOME_MM_UBICACION	1014  //rango (mm)*1e6-0.5e6
#define M_VALUE_HOME_PRIORIDAD		1015  //rango 0-15

#define M_VALUE_COORDENADAS_ABS 	90
#define M_VALUE_COORDENADAS_REL 	91
#define M_VALUE_ENABLE_MOTORS		18
#define M_VALUE_DISABLE_MOTORS		17
#define M_VALUE_PRINT_COORDENADAS	100

//G commands
#define G_VALUE_MOV_MAX_VEL			0
#define G_VALUE_MOV_F_VEL			1
#define G_VALUE_HOME_SECUENCIA		28
#define G_VALUE_COORDENADAS_ABS		90
#define G_VALUE_COORDENADAS_REL		91
#define G_VALUE_SET_POSICION_ABS	92

/*
********************************************************************************
*                   DATA TYPES
********************************************************************************
*/
struct eje_str{
	//PERIFERICOS
	uint16_t 	dir_pin;				//dir pin of axis, 	port B
	uint16_t 	step_pin;				//step pin of axis, port B
	uint16_t 	home_pin;				//home pin of axis, port A

	//VALORES EJE
	int64_t 	steps_actual;			//[step]	valor actual de step
	int64_t 	steps_goto;				//[step]	valor de steps al que se va a mover
	uint64_t 	v_max;					//[mm/s] 	velocidad maxima permitida por el eje
	uint64_t 	v_mov;					//[mm/s] 	velocidad a la que se va a realizar el movimiento actual
	uint64_t 	acc;					//[mm/s^2] 	aceleracion maxima permitida por el eje
	uint32_t 	porcentaje_v_min; 		//[%] 		de 0 a 100 porcentaje de v_max
	double 		step_mm;				//[step/mm] resolution axis
	uint8_t 	invertir_direccion;		//[bool]	booleana
	uint8_t 	char_name; 				//[char]	Name of axis

	//HOME
	uint8_t 	home_act;				//[bool]	booleana
	uint8_t 	home_dir;				//[bool]	booleana
	uint8_t 	home_prioridad;			//(0-255)	menor numero, mayor prioridad
	double 		home_mm_retroceso;		//[mm]		valor en mm que retrocede el eje al encontrar el home
	uint16_t 	home_porcentaje_vel;	//[%]		porcentaje de velocidad maxima a la que busca el home
	double 		home_ubic_mm;			//[mm]		valor en mm en la cual se sete el eje luego de la rutina de homming

	//RUN MOTORS V3
	uint64_t 	f_max_v3;
	uint64_t 	f_acc_v3;
	uint64_t 	f_min_v3;
	uint64_t 	N_loop_acc_v3;
	uint64_t 	N_loop_desacc_v3;
	uint64_t 	distancia_v3;
	uint64_t 	delta_v3;
	uint64_t 	f_actual_v3;
	uint64_t 	f_cresta_v3;
	uint64_t 	step_1_v3;
	uint64_t 	step_3_v3;
	uint64_t 	error_v3;
	uint32_t 	null_operation_v3;
};


struct G_code_str{
	uint8_t 			relative;

	uint16_t 			G_value;
	uint8_t 			new_G_value;
	uint16_t 			M_value;
	uint8_t 			new_M_value;
	uint64_t 			F_value;
	uint8_t 			new_F_value;

	uint8_t				N_ej;
	double 				ej_value[10];
	uint8_t 			char_ej_value[10];
	uint8_t 			new_ej_value[10];
	struct eje_str 		*p_eje[10];
};

/*
********************************************************************************
*                   PROTOTYPES
********************************************************************************
*/
void 		morcom_init(void);
void		morcom_run(void);
void 		morcom_read_line(void);
void		print_double(double);
void 		print_G0_fin_and_coordenadas(void);
void 		get_G_code(uint16_t, uint8_t *, uint8_t *, int32_t *, double *);
uint8_t 	validate_G_code(uint16_t, uint8_t *);
void 		reset_G_code(struct G_code_str *);
void 		set_default_RAM_values(void);
uint8_t 	validate_interface_message(uint8_t *);
void 		vector_to_interface(void);
void 		set_vector(uint32_t, uint32_t );
void 		flash_to_ram(void);
void 		vector_to_flash(void);
void 		vector_to_RAM(void);
void 		RAM_to_vector(void);
uint32_t 	HEX_string_to_uint32(uint8_t *, uint16_t, uint16_t);
uint8_t 	reconocer_caracter_HEX(uint8_t);



#endif /* MORCOM_V2_H */

/*** end of file ***/
