/*
 * EasyUART.h
 *
 *  Created on: Jun, 2022
 *      Author: Santiago Cian, santiagocian97@gmail.com
 *      		https://github.com/SantiagoCian1997/Easy_UART_STM32
 */

#ifndef EASYUART_H
#define EASYUART_H

#include "stm32f1xx_hal.h"

/*
********************************************************************************
*                   DEFINE CONSTANTS
********************************************************************************
*/
#define MULTI_PORT 0 			//0:false, 1:true
#define N_MULTI_PORT 3			//numbers of ports implemented
#define SIZE_BUFFERS 256		//size of all line
#define BUFFER_LINES 16			//numbers of lines stored for each port

/*
********************************************************************************
*                   DATA TYPES
********************************************************************************
*/
struct port_str{
	UART_HandleTypeDef *port;
	uint8_t inChar;
	uint8_t buffer[BUFFER_LINES][SIZE_BUFFERS];
	uint16_t sizeLine[BUFFER_LINES];				//tqamaño de cada linea
	uint16_t NWordsInLine[BUFFER_LINES];			//cantidad de palabras que contiene la linea
	uint16_t initActualWordsInLine[BUFFER_LINES];	//inicio de la palabra la ultima palabra que se leyo
	uint16_t actualLine;							//line en la que se esta escribiendo actualmente
	uint16_t lastLine;								//ultima linea solicitada por getLine
	uint16_t indexBuffer;							//elemento actual que se esta escribiendo
	uint16_t lastWord;								//almacena la ulitma palabra enviada
};

/*
********************************************************************************
*                   PROTOTYPES
********************************************************************************
*/
void 		EU_init(UART_HandleTypeDef *);
uint8_t 	EU_lineAvailable();
uint8_t 	*EU_getLine(uint16_t *);
uint16_t 	EU_getNWords();
uint8_t 	*EU_getNextWord(uint16_t *);
int32_t 	stringDecToInt(uint8_t *,int16_t );
uint64_t 	stringHexToInt(uint8_t *,int16_t );
double 		stringDecToDouble(uint8_t *,int16_t );

#if MULTI_PORT		//MULTI PORT FUNCTION
void 		EUM_init(uint8_t ,UART_HandleTypeDef *);
uint8_t 	EUM_lineAvailable(uint8_t);
uint8_t 	*EUM_getLine(uint8_t ,uint16_t *);
uint16_t 	EUM_getNWords(uint8_t);
uint8_t 	*EUM_getNextWord(uint8_t ,uint16_t *);
void 		setOutPort(uint8_t);
#endif

uint8_t 	thisCharIsANumberDEC(uint8_t);
uint8_t 	thisCharIsANumberHEX(uint8_t);
uint8_t 	thisCharIsANumberDECOrSignedOrPoint(uint8_t);
uint16_t 	charToValue_DEC_HEX(uint8_t );

#endif /* EASYUART_H */

/*** end of file ***/
