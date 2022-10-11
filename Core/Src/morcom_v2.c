/*
 * morcom_v2.c
 *
 *   Created on: May, 2022
 *       Author: Cian, Santiago Jose -- santiagocian97@gmail.com
 *               https://github.com/SantiagoCian1997/Morcom_v2
 *
 * 	Description:
 */

#include "morcom_v2.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"
#include "main.h"

#include "EasyUART.h"
#include "run_m.h"
#include "home_m.h"

extern UART_HandleTypeDef huart1;

struct eje_str X,Y,Z,B,C;
struct eje_str *p_ejes[N_MOTORS];
struct G_code_str G_code;

/* Description:
 *			function in charge of implementing all the control of Morcom_v2
 * Parameters:
 *			none
 * Return:
 *
 * Notes:
 *			must be called in a while(1) loop
 */
void morcom_run(){
	if(EU_lineAvailable()){
		morcom_read_line();
	}
}

/* Description:
 *			it is responsible for reading each line, it does so through the EasyUART.h utilities
 * Parameters:
 *			none
 * Return:
 *
 * Notes:
 *
 */
void morcom_read_line() {
	uint8_t N_palabras = EU_getNWords();
	uint16_t tamano_palabra;
	uint8_t *palabra_actual = EU_getNextWord(&tamano_palabra);
	if (0) {
		//future implementation
	} else {
		reset_G_code(&G_code);

		uint8_t char_command;
		int32_t int_command;
		double  dou_command;
		for (uint8_t LL = 0; LL < N_palabras; LL++) {
			if (validate_G_code(tamano_palabra, palabra_actual)) {
				get_G_code(tamano_palabra, palabra_actual, &char_command, &int_command, &dou_command);

				if (char_command == 'G') {
					G_code.G_value = int_command;
					G_code.new_G_value = 1;
				} else if (char_command == 'M') {
					G_code.M_value = int_command;
					G_code.new_M_value = 1;
				} else if (char_command == 'F') {
					G_code.F_value = int_command;
					G_code.new_F_value = 1;
				} else {
					G_code.ej_value[G_code.N_ej] = dou_command;
					G_code.char_ej_value[G_code.N_ej] = char_command;
					G_code.new_ej_value[G_code.N_ej] = 1;

					for (uint8_t eje = 0; eje < N_MOTORS; eje++) {
						if (G_code.char_ej_value[G_code.N_ej] == (*p_ejes[eje]).char_name) G_code.p_eje[G_code.N_ej] = p_ejes[eje];
					}

					G_code.N_ej++;
					if (G_code.N_ej >= 10) G_code.N_ej = 0;
				}
			}
			palabra_actual = EU_getNextWord(&tamano_palabra);
		}

		printf("estado G_code:\n");
		if (G_code.new_G_value)printf("    G%i\n", G_code.G_value);
		if (G_code.new_M_value)printf("    M%i\n", G_code.M_value);
		if (G_code.new_F_value)printf("    F%i\n", (int)G_code.F_value);
		for (uint8_t i = 0; i < G_code.N_ej; i++) {
			printf("    %c", (*G_code.p_eje[i]).char_name);
			print_double(G_code.ej_value[i]);
			printf("\n");
		}

		if ((G_code.G_value == G_VALUE_MOV_MAX_VEL || G_code.G_value == G_VALUE_MOV_F_VEL) && G_code.new_M_value == 0) { //condicion para que haya movimiento

			if (G_code.F_value > 0 || G_code.G_value == G_VALUE_MOV_MAX_VEL) { //si se mueve a G0 o a G1 con F>0
				uint8_t inter_XY = 0, inter_ZB = 0;
				for (uint8_t i = 0; i < G_code.N_ej; i++) {
					if (G_code.G_value == G_VALUE_MOV_MAX_VEL || (*G_code.p_eje[i]).v_max < G_code.F_value)  (*G_code.p_eje[i]).v_mov = (*G_code.p_eje[i]).v_max; //movimiento a v max
					else                                        (*G_code.p_eje[i]).v_mov = G_code.F_value;    //movimiento a v=F_value

					if (G_code.relative == 0)        (*G_code.p_eje[i]).steps_goto = G_code.ej_value[i] * (*G_code.p_eje[i]).step_mm; //asigno la posicion final del motor en modo absoluto
					else                (*G_code.p_eje[i]).steps_goto += G_code.ej_value[i] * (*G_code.p_eje[i]).step_mm; //asigno la posicion final del motor en modo relativo

					if ((*G_code.p_eje[i]).char_name == 'X' || (*G_code.p_eje[i]).char_name == 'Y') inter_XY++;
					if ((*G_code.p_eje[i]).char_name == 'Z' || (*G_code.p_eje[i]).char_name == 'B') inter_ZB++;
				}
				if (inter_XY >= 2) {
					int64_t delta_X = X.steps_actual - X.steps_goto;
					int64_t delta_Y = Y.steps_actual - Y.steps_goto;
					if (delta_X < 0) delta_X = -delta_X;
					if (delta_Y < 0) delta_Y = -delta_Y;
					if (delta_X > delta_Y && delta_X < 10 * delta_Y)  Y.v_mov = (uint64_t)(((double)(Y.v_mov * delta_Y)) / ((double)delta_X));
					if (delta_Y > delta_X && delta_Y < 10 * delta_X)  X.v_mov = (uint64_t)(((double)(X.v_mov * delta_X)) / ((double)delta_Y));
				}
				run_motors();
				print_G0_fin_and_coordenadas();
			}
		}
		else { //no debe hacer movimiento
			if (G_code.new_G_value) {
				switch (G_code.G_value) {
				case G_VALUE_COORDENADAS_ABS: {       //modo coordenadas absolutas
					G_code.relative = 0;
					print_G0_fin_and_coordenadas();
				} break;
				case G_VALUE_COORDENADAS_REL: {       //modo coordenadas relativas
					G_code.relative = 1;
					print_G0_fin_and_coordenadas();
				} break;
				case G_VALUE_HOME_SECUENCIA: {        //secuencia de home
					home_sequence();
					print_G0_fin_and_coordenadas();
				} break;
				case G_VALUE_SET_POSICION_ABS: {      //set posicion absoluta del sistema de coordenadas
					for (uint8_t i = 0; i < G_code.N_ej; i++) {
						(*G_code.p_eje[i]).steps_actual = G_code.ej_value[i] * (*G_code.p_eje[i]).step_mm;
						(*G_code.p_eje[i]).steps_goto = (*G_code.p_eje[i]).steps_actual;
					}
				} break;
				default : {} break;
				}
			}
			if (G_code.new_M_value) {
				switch (G_code.M_value) {
				case M_VALUE_COORDENADAS_ABS: {       //modo coordenadas absolutas
					G_code.relative = 0;
					print_G0_fin_and_coordenadas();
				} break;
				case M_VALUE_COORDENADAS_REL: {       //modo coordenadas relativas
					G_code.relative = 1;
					print_G0_fin_and_coordenadas();
				} break;
				case M_VALUE_ENABLE_MOTORS: {
					HAL_GPIO_WritePin(Enable_motors_GPIO_Port, Enable_motors_Pin, 0); //encender
					print_G0_fin_and_coordenadas();
				} break;
				case M_VALUE_DISABLE_MOTORS: {
					HAL_GPIO_WritePin(Enable_motors_GPIO_Port, Enable_motors_Pin, 1); //apagar motores
					print_G0_fin_and_coordenadas();
				} break;
				case M_VALUE_PRINT_COORDENADAS: {
					print_G0_fin_and_coordenadas();
				} break;
				case M_VALUE_CONSULTA_SETS: {
					printf("--Parametros actuales--\n");
					printf("EJE  |V_max     |Acc       |step/mm    |%c_vel_min |neg_dir |\n", '%');
					for (uint8_t i = 0; i < N_MOTORS; i++) {
						printf("%c    |%10i|%10i|", (*p_ejes[i]).char_name, (int)(*p_ejes[i]).v_max, (int)(*p_ejes[i]).acc); print_double((*p_ejes[i]).step_mm); printf("|%10i|   %3i  |\n", (int)(*p_ejes[i]).porcentaje_v_min, (int)(*p_ejes[i]).invertir_direccion);
					}
					printf("HOME |act|dir|%c_vel|retroceso[mm]|ubicacion[mm]|prioridad|\n", '%');
					for (uint8_t i = 0; i < N_MOTORS; i++) {
						printf("%c    | %1i | %1i | %3i |  ", (*p_ejes[i]).char_name, (*p_ejes[i]).home_act, (*p_ejes[i]).home_dir, (*p_ejes[i]).home_porcentaje_vel); print_double((*p_ejes[i]).home_mm_retroceso); printf("|"); print_double((*p_ejes[i]).home_ubic_mm);  printf("|     %3i|\n", (*p_ejes[i]).home_prioridad);
					}
					print_G0_fin_and_coordenadas();
				} break;
				case M_VALUE_SAVE_SETS: {
					RAM_to_vector();
					vector_to_flash();
					printf("Flash set\n");
				} break;
				default: {
					for (uint8_t i = 0; i < G_code.N_ej; i++) {
						switch (G_code.M_value) {
						case M_VALUE_EJE_VEL_MAX:       (*G_code.p_eje[i]).v_max =					G_code.ej_value[i];       break;
						case M_VALUE_EJE_ACC:	        (*G_code.p_eje[i]).acc =					G_code.ej_value[i];       break;
						case M_VALUE_EJE_STEP_MM:       (*G_code.p_eje[i]).step_mm =				G_code.ej_value[i];       break;
						case M_VALUE_EJE_PERC_VEL:      (*G_code.p_eje[i]).porcentaje_v_min =		G_code.ej_value[i];       break;
						case M_VALUE_EJE_NEG_DIR:       (*G_code.p_eje[i]).invertir_direccion =		G_code.ej_value[i] > 0;   break;
						case M_VALUE_HOME_ACT_EJE:      (*G_code.p_eje[i]).home_act =				G_code.ej_value[i] > 0;   break;
						case M_VALUE_HOME_DIR_EJE:      (*G_code.p_eje[i]).home_dir =				G_code.ej_value[i] > 0;   break;
						case M_VALUE_HOME_PERC_VEL:     (*G_code.p_eje[i]).home_porcentaje_vel =	G_code.ej_value[i];       break;
						case M_VALUE_HOME_MM_RETROCESO: (*G_code.p_eje[i]).home_mm_retroceso =		G_code.ej_value[i];       break;
						case M_VALUE_HOME_MM_UBICACION: (*G_code.p_eje[i]).home_ubic_mm =			G_code.ej_value[i];       break;
						case M_VALUE_HOME_PRIORIDAD:    (*G_code.p_eje[i]).home_prioridad =			G_code.ej_value[i];       break;
						default : break;
						}
					}
					print_G0_fin_and_coordenadas();
				} break;
				}
			}
		}
	}
	EU_getLine(NULL);
}


/* Description:
 *			initialization of axis parameters and the serial port
 * Parameters:
 *			none
 * Return:
 *
 * Notes:
 *			call it on boot
 */
void morcom_init() {
  HAL_GPIO_WritePin(Enable_motors_GPIO_Port, Enable_motors_Pin, 1);//apagar motores
  EU_init(&huart1);
  printf("INIR MORCOM V2\n\n    send M999 for help and axes states\n");

  p_ejes[0] = &X;
  X.step_pin = Step_X_Pin; //puerto B
  X.dir_pin = Dir_X_Pin;  //puerto B
  X.home_pin = Home_X_Pin; //puerto A
  X.char_name = 'X';

  p_ejes[1] = &Y;
  Y.step_pin = Step_Y_Pin;
  Y.dir_pin = Dir_Y_Pin;
  Y.home_pin = Home_Y_Pin;
  Y.char_name = 'Y';

  p_ejes[2] = &Z;
  Z.step_pin = Step_Z_Pin;
  Z.dir_pin = Dir_Z_Pin;
  Z.home_pin = Home_Z_Pin;
  Z.char_name = 'Z';

  p_ejes[3] = &B;
  B.step_pin = Step_B_Pin;
  B.dir_pin = Dir_B_Pin;
  B.home_pin = Home_B_Pin;
  B.char_name = 'B';

  p_ejes[4] = &C;
  C.step_pin = Step_C_Pin;
  C.dir_pin = Dir_C_Pin;
  C.home_pin = Home_C_Pin;
  C.char_name = 'C';

  reset_G_code(&G_code);

  flash_to_ram();
  vector_to_RAM();

  if (X.home_prioridad == 255) { //la flash nunca fue grabada
    set_default_RAM_values();
    RAM_to_vector();
    vector_to_flash();
  }
}

/* Description:
 *				get G code commands
 * Parameters:
 *				uint16_t tamano:		word size
 *				uint8_t *palabra:		pointer to start of word
 *				uint8_t *char_return:	pointer of the variable where it returns the letter of the parsed command
 *				int32_t *int_return:	pointer to the variable where it returns the integer of the parsed command
 *				double *dou_return:		pointer to the variable where it returns the double of the parsed command
 * Return:
 *
 * Notes:
 *			internal use
 */
void get_G_code(uint16_t tamano, uint8_t *palabra, uint8_t *char_return, int32_t *int_return, double *dou_return){
	(*char_return) = palabra[0];
	uint8_t negative = 0;
	uint8_t i = 1;
	if(palabra[1]=='-' || palabra[1]=='+'){
		i = 2;
		if(palabra[1] == '-') negative = 1;
	}
	uint32_t decimal_part=0;
	(*int_return) = 0;
	(*dou_return) = 0;
	while (i < tamano){
		if(palabra[i] == '.'){
			decimal_part = 1;
			i++;
		}
		if(i < tamano){
			if(decimal_part == 0){
				(*int_return) *= 10;
				(*int_return) += palabra[i] - '0';
				(*dou_return) = (*int_return);
			}
			else{
				decimal_part *= 10;
				(*dou_return) += ((double)(palabra[i]-'0')) / ((double)decimal_part);
			}
			i++;
		}
	}
	if(negative==1){
		(*int_return) = -(*int_return);
		(*dou_return) = -(*dou_return);
	}
}

/* Description:
 *				function in charge of validating the G code command
 * Parameters:
 *				uint16_t tamano:		word size
 *				uint8_t *palabra:		pointer to start of word
 * Return:
 *				uint8_t : 1 if the command is valid, else 0
 * Notes:
 *			internal use
 */
uint8_t validate_G_code(uint16_t tamano, uint8_t *palabra){
	uint8_t letter = 0;
	if(palabra[0] >= 'A' && palabra[0] <= 'Z'){
		letter = palabra[0];
		uint16_t i = 1;

		if(palabra[1] == '-' || palabra[1] == '+') i=2;

		while (i < tamano){
			if(!(palabra[i] >= '0' && palabra[i] <= '9')){
				if(palabra[i] != '.') return(0);
				else i++;
			}
			i++;
		}
	}
	if(letter != 0) return(1);
	return(0);
}



/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
uint8_t  validate_interface_message(uint8_t *buff){
	uint8_t i=0;
	uint8_t cont_aperturas_llaves = 0;
	uint8_t cont_cierre_llaves = 0;
	uint8_t cont_arrobas = 0;
	while(buff[i] >= 32){
		if(buff[i] == '{') cont_aperturas_llaves++;
		if(buff[i] == '}') cont_cierre_llaves++;
		if(buff[i] == '@') cont_arrobas++;
		i++;
	}
	if(cont_aperturas_llaves == 1 && cont_cierre_llaves == 1 && cont_arrobas == 1) return(1); //Solo si hay UNA '{', UNA '}' y UNA '@' el mensaje es valido
	return(0);
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void print_double(double val){
	if(val < 0) {
		val = -val;
		printf("-");
	}
	printf("%04i.%06i", (int)val, (int)((val-(int)val)*1000000));
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *
 */
uint32_t HEX_string_to_uint32(uint8_t *buff, uint16_t inicio, uint16_t fin) { //devuelve el valor numerico de los HEX presentes en la linea entre los indices dados
  uint32_t ret = 0;
  while (reconocer_caracter_HEX(buff[inicio]) >= 0 && inicio <= fin) {
    ret = ret << 4;
    ret += reconocer_caracter_HEX(buff[inicio]);
    inicio++;
  }
  return (ret);
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
uint8_t reconocer_caracter_HEX(uint8_t value) { //si es un caracter HEX retorna su valor (0-15)
  if (value >= '0' && value <= '9') return (value - 48);
  if (value >= 'A' && value <= 'F') return (value - 55);
  if (value >= 'a' && value <= 'f') return (value - 87);
  return (-1);
}


/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void reset_G_code(struct G_code_str *G){
	(*G).new_G_value = 0;
	(*G).new_M_value = 0;
	(*G).new_F_value = 0;
	(*G).N_ej = 0;
	for(uint8_t i=0; i<10; i++){
		(*G).ej_value[i] = 0;
		(*G).char_ej_value[i] = 0;
		(*G).new_ej_value[i] = 0;
	}
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
#define size_vector_ejes 25
#define size_vector_home 35
uint32_t vector_interfaz_ejes[size_vector_ejes];
uint32_t vector_interfaz_home[size_vector_home];

void vector_to_interface(){
	printf("{%X@",INDEX_EJE_VEL_MAX);
	for(uint8_t i=0	; i<size_vector_ejes ; i++) printf("%X:",(int)vector_interfaz_ejes[i]);
	printf("}\n");
	printf("{%X@",INDEX_HOME_ACTIVO);
	for(uint8_t i=0	; i<size_vector_home ; i++) printf("%X:",(int)vector_interfaz_home[i]);
	printf("}\n");
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void flash_to_ram(){
	for(uint8_t i=0					; i<size_vector_ejes				  ; i++) vector_interfaz_ejes[i] = 					*(uint32_t*)(inicioPagina_flash + i*4);//Read
	for(uint8_t i=size_vector_ejes	; i<size_vector_home+size_vector_ejes ; i++) vector_interfaz_home[i-size_vector_ejes] =	*(uint32_t*)(inicioPagina_flash + i*4);//Read
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void vector_to_RAM(){
	for(uint8_t i=0; i<N_MOTORS; i++){
		(*p_ejes[i]).v_max					=vector_interfaz_ejes[0+i];
		(*p_ejes[i]).acc					=vector_interfaz_ejes[5+i];
		(*p_ejes[i]).step_mm				=((double)vector_interfaz_ejes[10+i])/1000000;
		(*p_ejes[i]).porcentaje_v_min		=vector_interfaz_ejes[15+i];
		(*p_ejes[i]).invertir_direccion		=vector_interfaz_ejes[20+i];

		(*p_ejes[i]).home_act				=vector_interfaz_home[0+i];
		(*p_ejes[i]).home_dir				=vector_interfaz_home[5+i];
		(*p_ejes[i]).home_porcentaje_vel	=vector_interfaz_home[10+i];
		(*p_ejes[i]).home_mm_retroceso		=vector_interfaz_home[15+i];
		(*p_ejes[i]).home_ubic_mm			=((double)(vector_interfaz_home[20+i]-500000))/1000000;
		(*p_ejes[i]).home_prioridad			=vector_interfaz_home[25+i];
	}
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void RAM_to_vector(){
	for(uint8_t i=0;i<N_MOTORS;i++){
		vector_interfaz_ejes[0+i]	=					(*p_ejes[i]).v_max;
		vector_interfaz_ejes[5+i]	=					(*p_ejes[i]).acc;
		vector_interfaz_ejes[10+i]	=					(uint32_t)((*p_ejes[i]).step_mm*1000000);
		vector_interfaz_ejes[15+i]	=					(*p_ejes[i]).porcentaje_v_min;
		vector_interfaz_ejes[20+i]	=					(*p_ejes[i]).invertir_direccion;

		vector_interfaz_home[0+i]	=					(*p_ejes[i]).home_act;
		vector_interfaz_home[5+i]	=					(*p_ejes[i]).home_dir;
		vector_interfaz_home[10+i]	=					(*p_ejes[i]).home_porcentaje_vel;
		vector_interfaz_home[15+i]	=					(*p_ejes[i]).home_mm_retroceso;
		vector_interfaz_home[20+i]	=					((uint32_t)((*p_ejes[i]).home_ubic_mm*1000000))-500000;
		vector_interfaz_home[25+i]	=					(*p_ejes[i]).home_prioridad;
	}
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void vector_to_flash(){
	uint32_t ErrorPagina;
	//HAL_StatusTypeDef status;
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef eraseStruct;
	eraseStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseStruct.PageAddress = inicioPagina_flash;
	eraseStruct.NbPages = 3;
	HAL_FLASHEx_Erase(&eraseStruct, &ErrorPagina);
	for(uint8_t i=0					; i<size_vector_ejes				  ; i++) HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, inicioPagina_flash + i*4,vector_interfaz_ejes[i]);  //Write
	for(uint8_t i=size_vector_ejes	; i<size_vector_home+size_vector_ejes ; i++) HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, inicioPagina_flash + i*4,vector_interfaz_home[i - size_vector_ejes]);  //Write
	HAL_FLASH_Lock();
	printf("flash seteada\n");
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void set_vector(uint32_t index,uint32_t value){
	printf("mensaje_interfaz:\n");
	printf("		Indice: 0x%X = %i\n",(uint16_t)index,(uint16_t)index);
	printf("		Value : 0x%X = %i\n",(uint16_t)value,(uint16_t)value);
	if(index >= INDEX_EJE_VEL_MAX && index < INDEX_EJE_VEL_MAX+size_vector_ejes) vector_interfaz_ejes[index-INDEX_EJE_VEL_MAX]=value;
	if(index >= INDEX_HOME_ACTIVO && index < INDEX_HOME_ACTIVO+size_vector_home) vector_interfaz_home[index-INDEX_HOME_ACTIVO]=value;
	if(index == INDEX_N_VERSION_MORCOM){}//AGREGAR RETURN DE VERSION!
	if(index == INDEX_RETURN_VECTOR_MORCOM){
		flash_to_ram();
		vector_to_interface();
	}
	if(index == INDEX_APLICAR_CAMBIOS_MORCOM){
		vector_to_RAM();
	}
	if(index == INDEX_GUARDAR_VECTOR_EN_FLASH_MORCOM){
		vector_to_RAM();
		vector_to_flash();
	}
	if(index == INDEX_FIN_RETURN_VECTOR_MORCOM){
		//No se usa, en este index morcom retorna a la interfaz
	}
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void print_G0_fin_and_coordenadas(){
	if(G_code.relative == 0) printf("Abs.");
	else printf("Rel.");
	for(uint8_t i=0; i<N_MOTORS; i++){
		printf(" %c", (*p_ejes[i]).char_name);
		print_double(((double)(*p_ejes[i]).steps_actual)/(*p_ejes[i]).step_mm);

	}
	printf("\nG0_fin\n");
}

/* Description:
 *
 * Parameters:
 *
 * Return:
 *
 * Notes:
 *			internal use
 */
void set_default_RAM_values(){
	for(uint8_t i=0; i<N_MOTORS; i++){
		(*p_ejes[i]).v_max =					10000;
		(*p_ejes[i]).acc =						5000;
		(*p_ejes[i]).porcentaje_v_min =			2;
		(*p_ejes[i]).step_mm =					1;
		(*p_ejes[i]).invertir_direccion =		0;
		(*p_ejes[i]).home_act =					0;
		(*p_ejes[i]).home_dir =					0;
		(*p_ejes[i]).home_prioridad =			0;
		(*p_ejes[i]).home_mm_retroceso =		20;
		(*p_ejes[i]).home_porcentaje_vel =		35;
		(*p_ejes[i]).home_ubic_mm =				0;
	}
}

/*** end of file ***/
