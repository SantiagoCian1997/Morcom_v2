/*
 * EasyUART.c
 *
 *  Created on: Jun 17, 2022
 *      Author: Santiago Cian, santiagocian97@gmail.com
 *      		https://github.com/SantiagoCian1997/Easy_UART_STM32
 */
#include "EasyUART.h"



#if MULTI_PORT
	struct port_str ports[N_MULTI_PORT];
#else
	struct port_str ports[1];
#endif

struct port_str * port_selected=ports;
struct port_str * port_read=ports;
struct port_str * port_write=ports;


/* Description: Create interruption for listen each port
 *
 * Parameters:
 * 				UART_HandleTypeDef *huart: struct of port UART (see HAL library)
 * Return:
 * 				none
 * Notes:
 *
 */
void EU_init(UART_HandleTypeDef *huart){
	(*port_selected).port=huart;
	HAL_UART_Receive_IT((*port_selected).port, &(*port_selected).inChar, 1);
	(*port_selected).actualLine=0;
	(*port_selected).lastLine=0;
	(*port_selected).indexBuffer=0;
}

/* Description: Confirm if there is aline to read
 *
 * Parameters:
 * 				none
 * Return:
 * 				1: if exist a new line for read
 * 				0: if don´t exist
 * Notes:
 *
 */
uint8_t EU_lineAvailable(){
	if((*port_selected).lastLine!=(*port_selected).actualLine){
		return (1);
	}
	return (0);
}


/* Description: If exist line for read, return pointer of buffer and modifies the line of the buffer that is read
 *
 * Parameters:
 * 				uint16_t *size: returns here the length of the line
 * Return:
 * 				uint8_t *: the pointer to init line
 * 				if no exist line return NULL=0;
 * Notes:
 *
 */
uint8_t *EU_getLine(uint16_t *size){
	if(EU_lineAvailable()){
		(*size)=(*port_selected).sizeLine[(*port_selected).lastLine];
		uint8_t *buff=((*port_selected).buffer[(*port_selected).lastLine]);
		(*port_selected).lastLine++;
		if((*port_selected).lastLine>=BUFFER_LINES)(*port_selected).lastLine=0;
		return(buff);
	}
	return(0);
}

/* Description: Returns number of word existin in buffer line (last line)
 *
 * Parameters:
 * 				none
 * Return:
 * 				uint16_t : number of word existin
 * Notes:
 *
 */
uint16_t EU_getNWords(){
	uint16_t indice=0;
	uint16_t NWords=0;
	(*port_selected).initActualWordsInLine[(*port_selected).lastLine]=0;
	while((*port_selected).buffer[(*port_selected).lastLine][indice] > ' ' && indice < (*port_selected).sizeLine[(*port_selected).lastLine]){
		NWords++;
		while((*port_selected).buffer[(*port_selected).lastLine][indice++] >  ' ');
		while((*port_selected).buffer[(*port_selected).lastLine][indice] == ' ') indice++; //para detectar mas de un espacio
	}
	(*port_selected).NWordsInLine[(*port_selected).lastLine]=NWords;
	return (NWords);
}


/* Description: Returns pointer to init word and length of this
 *
 * Parameters:
 * 				uint16_t *size: returns here the length of the word
 * Return:
 * 				uint8_t *: the pointer to init word
 * Notes:
 *
 */
uint8_t *EU_getNextWord(uint16_t *size){
	uint16_t inicio=(*port_selected).initActualWordsInLine[(*port_selected).lastLine];
	uint16_t fin=inicio;
	uint16_t space=0;
	while ((*port_selected).buffer[(*port_selected).lastLine][fin] >  ' ')fin++;
	uint16_t aux_fin=fin;
	while ((*port_selected).buffer[(*port_selected).lastLine][aux_fin] == ' '){ //para detectar mas de un espacio
		space++;
		aux_fin++;
	}
	(*port_selected).initActualWordsInLine[(*port_selected).lastLine]=fin+space;
	(*size)=fin-inicio;
	return(&(*port_selected).buffer[(*port_selected).lastLine][inicio]);
}


/** HAL library, interruption for UART
 *
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
uint8_t index_port=0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
#if MULTI_PORT
	if(huart!=(*port_read).port){
		while(huart!=ports[index_port].port)	index_port=(index_port+1)%N_MULTI_PORT;
		port_read=&ports[index_port];
	}
#endif
	if((*port_read).inChar<' '){
		(*port_read).sizeLine[(*port_read).actualLine]=(*port_read).indexBuffer;
		(*port_read).indexBuffer=0;
		(*port_read).actualLine++;
		if((*port_read).actualLine>=BUFFER_LINES)(*port_read).actualLine=0;
	}else{
		if((*port_read).indexBuffer<SIZE_BUFFERS-1){
			(*port_read).buffer[(*port_read).actualLine][(*port_read).indexBuffer++]=(*port_read).inChar;
			(*port_read).buffer[(*port_read).actualLine][(*port_read).indexBuffer]=0;
		}
	}
	HAL_UART_Receive_IT((*port_read).port, &(*port_read).inChar, 1);
}



/* for printf function
 *
 */
int __io_putchar(int ch){
	HAL_UART_Transmit((*port_write).port, (uint8_t *)&ch, 1, 100);
	return ch;
}
//////////////////   MULTIPLES PUERTOS
#if MULTI_PORT

/* Description: Create interruption for listen each port
 *
 * Parameters:
 * 				uint8_t p: number of port
 * 				UART_HandleTypeDef *huart: struct of port UART (see HAL library)
 * Return:
 * 				none
 * Notes:
 * 				hem
 */
void EUM_init(uint8_t p ,UART_HandleTypeDef *huart){
	port_selected=&ports[p%N_MULTI_PORT];
	EU_init(huart);
}

/* Description: Confirm if there is aline to read
 *
 * Parameters:
 * 				uint8_t p: number of port
 * 				none
 * Return:
 * 				1: if exist a new line for read
 * 				0: if don´t exist
 * Notes:
 *
 */
uint8_t EUM_lineAvailable(uint8_t p){
	port_selected=&ports[p%N_MULTI_PORT];
	return (EU_lineAvailable());
}


/* Description: If exist line for read, return pointer of buffer and modifies the line of the buffer that is read
 *
 * Parameters:
 * 				uint8_t p: number of port
 * 				uint16_t *size: returns here the length of the line
 * Return:
 * 				uint8_t *: the pointer to init line
 * 				if no exist line return NULL=0;
 * Notes:
 *
 */
uint8_t *EUM_getLine(uint8_t p ,uint16_t *size){
	port_selected=&ports[p%N_MULTI_PORT];
	return(EU_getLine(size));
}

/* Description: Returns number of word existin in buffer line (last line)
 *
 * Parameters:
 * 				uint8_t p: number of port
 * 				none
 * Return:
 * 				uint16_t : number of word existin
 * Notes:
 *
 */
uint16_t EUM_getNWords(uint8_t p){
	port_selected=&ports[p%N_MULTI_PORT];
	return(EU_getNWords());
}


/* Description: Returns pointer to init word and length of this
 *
 * Parameters:
 * 				uint8_t p: number of port
 * 				uint16_t *size: returns here the length of the word
 * Return:
 * 				uint8_t *: the pointer to init word
 * Notes:
 *
 */
uint8_t *EUM_getNextWord(uint8_t p ,uint16_t *size){
	port_selected=&ports[p%N_MULTI_PORT];
	return(EU_getNextWord(size));
}


/* Description: Selec port OUTPUT
 *
 * Parameters:
 * 				uint8_t p: number of port
 * Return:
 * 				none
 * Notes:
 *
 */
void setOutPort(uint8_t p){
	port_write=&ports[p%N_MULTI_PORT];
}
#endif



/* Description: Converts a character array containing a decimal number to a 32-bit signed integer
 *
 * Parameters:
 *				uint8_t *buffer: pointer to the beginning of the number
 *				int16_t length: number of characters that contains the number
 * Return:
 *				int32_t
 * Notes:
 *			if you put length=0 it will convert the characters from the beginning to the first occurrence of an invalid character
 */
int32_t stringDecToInt(uint8_t *buffer,int16_t length){
	uint16_t inicio=0;
	int32_t ret=0;
	while(thisCharIsANumberDECOrSignedOrPoint(buffer[inicio])==0 && buffer[inicio]>' ') inicio++;
	uint8_t signo=0;
	if(buffer[inicio]=='-'){
		signo=1;
		inicio++;
	}
	if(thisCharIsANumberDEC(buffer[inicio])==1){
		if(length>0){ //busca hasta esta posicion
			while(length>0){
				length--;
				if(thisCharIsANumberDEC(buffer[inicio])==1) {
					ret*=10;
					ret+=charToValue_DEC_HEX(buffer[inicio]);
				}
				else length=0;
				inicio++;
			}
			if(signo) return(-ret);
			else return(ret);
		}else{	//busca hasta cualquier caracter que no sea numero DEC
			while(thisCharIsANumberDEC(buffer[inicio])){
				ret*=10;
				ret+=charToValue_DEC_HEX(buffer[inicio]);
				inicio++;
			}
			if(signo) return(-ret);
			else return(ret);

		}
	}
	return(0);
}

/* Description: Converts a character array containing a HEX number to a 64-bit unsigned integer
 *
 * Parameters:
 *				uint8_t *buffer: pointer to the beginning of the number
 *				int16_t length: number of characters that contains the number
 * Return:
 *				uint64_t
 * Notes:
 *			if you put length=0 it will convert the characters from the beginning to the first occurrence of an invalid character
 */
uint64_t stringHexToInt(uint8_t *buffer,int16_t length){
	uint16_t inicio=0;
	uint64_t ret=0;
	while(thisCharIsANumberHEX(buffer[inicio])==0 && buffer[inicio]>' ') inicio++;
	if(thisCharIsANumberHEX(buffer[inicio])==1){
		if(length>0){ //busca hasta esta posicion
			while(length>0){
				length--;
				if(thisCharIsANumberHEX(buffer[inicio])==1) {
					ret = ret<<4;
					ret += charToValue_DEC_HEX(buffer[inicio]);
				}
				else length=0;
				inicio++;
			}
			return(ret);
		}else{	//busca hasta cualquier caracter que no sea numero DEC
			while(thisCharIsANumberHEX(buffer[inicio])){
				ret = (ret<<4) | charToValue_DEC_HEX(buffer[inicio]);
				inicio++;
			}
			return(ret);
		}
	}
	return(0);
}

/* Description: Converts a character array containing a number whit decimal point to a double
 *
 * Parameters:
 *				uint8_t *buffer: pointer to the beginning of the number
 *				int16_t length: number of characters that contains the number
 * Return:
 *			double
 * Notes:
 *			if you put length=0 it will convert the characters from the beginning to the first occurrence of an invalid character
 */
double stringDecToDouble(uint8_t *buffer,int16_t length){
	double ret=0;
	uint8_t signo=0;
	uint16_t inicio=0;
	uint16_t N_decimal=0;
	while(thisCharIsANumberDECOrSignedOrPoint(buffer[inicio])==0 && buffer[inicio]>' ') inicio++;
	if(buffer[inicio]=='-'){
		signo=1;
		inicio++;
	}
	if(thisCharIsANumberDECOrSignedOrPoint(buffer[inicio])==1){
		if(length>0){ //busca hasta esta posicion
			while(length>0){
				length--;
				if(thisCharIsANumberDECOrSignedOrPoint(buffer[inicio])==1) {
					if(buffer[inicio]=='.'){
						N_decimal=1;
						inicio++;
					}
					if(N_decimal>0){
						N_decimal*=10;
						ret+=((double)charToValue_DEC_HEX(buffer[inicio]))/((double)N_decimal);
					}
					else{
						ret*=10;
						ret+=(double)charToValue_DEC_HEX(buffer[inicio]);
					}
				}
				else length=0;
				inicio++;
			}
			if(signo) return(-ret);
			else return(ret);
		}else{	//busca hasta cualquier caracter que no sea numero DEC
			while(thisCharIsANumberDECOrSignedOrPoint(buffer[inicio])==1) {
				if(buffer[inicio]=='.'){
					N_decimal=1;
					inicio++;
				}
				if(N_decimal>0){
					N_decimal*=10;
					ret+=((double)charToValue_DEC_HEX(buffer[inicio]))/((double)N_decimal);
				}
				else{
					ret*=10;
					ret+=(double)charToValue_DEC_HEX(buffer[inicio]);
				}
				inicio++;
			}
			if(signo) return(-ret);
			else return(ret);

		}
	}
	return(0);
}

uint8_t thisCharIsANumberDEC(uint8_t val){
	if(val>='0'&&val<='9') return(1);
	return(0);
}
uint8_t thisCharIsANumberHEX(uint8_t val){
	if(val>='0'&&val<='9') return(1);
	if(val>='A'&&val<='F') return(1);
	if(val>='a'&&val<='f') return(1);
	return(0);
}
uint8_t thisCharIsANumberDECOrSignedOrPoint(uint8_t val){
	if(val>='0'&&val<='9') return(1);
	if(val=='-') return(1);
	if(val=='.') return(1);
	//if(val=='+') return(1);
	return(0);
}
uint16_t charToValue_DEC_HEX(uint8_t val){
	if(val>='0'&&val<='9') return(val-'0');
	if(val>='A'&&val<='F') return(val-'A'+10);
	if(val>='a'&&val<='f') return(val-'a'+10);
	return(0);
}

/*** end of file ***/
