/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
#define tamano_buffer_UART 256
#define maxima_cant_lineas 10
uint8_t bufferUART[tamano_buffer_UART][maxima_cant_lineas];
uint8_t indiceBuferLINEA=0;
uint8_t indiceBuferUART=0;
uint8_t auxCaracterInSerie;
uint8_t home_inicio=0;
const uint8_t tamano_vector=70;
uint32_t vector_datos_interfaz[256];
float version_float = 1.0;
uint32_t inicioPagina_flash=0x08018000;//, fin en 0x0801fff0
uint8_t parada_de_emergencia=0;

struct eje_str X,Y,Z,B,C;
extern void run_motores(double,double);
extern void despulsar_eje(struct eje_str);
void despulsar_limite_arranque(struct eje_str );
void analizar_switch_arranque();
extern void home_secuencia();
extern void home_inicio_sec (uint8_t);
double string_to_double(uint8_t*,uint8_t);
void analizarLineaUART(uint8_t);
void print_G0_fin_and_coordenadas();
void print_double(double);
uint8_t _HEX_to_int(char );
void vector_to_RAM(void);
void vector_to_Flash(void);
void Flash_to_vector(void);
void print_vecotr(void);
int __io_putchar(int ch){//para el printf
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
	return ch;
}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(Enable_motors_GPIO_Port, Enable_motors_Pin, 1);//apagar motores
  HAL_UART_Receive_IT(&huart1, &auxCaracterInSerie, 1);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  Flash_to_vector();
  vector_to_RAM();
/*
  X.v_max=1500;
  X.acc=10000;
  X.porcentaje_v_min=2;
  X.step_mm=10;
  X.mm_actual=0;
  X.mm_goto=0;

  Y.v_max=1500;
  Y.acc=10000;
  Y.porcentaje_v_min=2;
  Y.step_mm=10;
  Y.mm_actual=0;
  Y.mm_goto=0;

  Z.v_max=1500;
  Z.acc=10000;
  Z.porcentaje_v_min=2;
  Z.step_mm=10;
  Z.mm_actual=0;
  Z.mm_goto=0;

  B.v_max=1500;
  B.acc=10000;
  B.porcentaje_v_min=12;
  B.step_mm=10;
  B.mm_actual=0;
  B.mm_goto=0;

  C.v_max=1500;
  C.acc=10000;
  C.porcentaje_v_min=12;
  C.step_mm=10;
  C.mm_actual=0;
  C.mm_goto=0;*/
  home_inicio_sec(home_inicio);
  uint8_t N_linea_actual=0;

  X.step_pin=Step_X_Pin;//puerto B
  X.dir_pin=Dir_X_Pin;//puerto B
  X.home_pin=Home_X_Pin;//puerto A

  Y.step_pin=Step_Y_Pin;
  Y.dir_pin=Dir_Y_Pin;
  Y.home_pin=Home_Y_Pin;

  Z.step_pin=Step_Z_Pin;
  Z.dir_pin=Dir_Z_Pin;
  Z.home_pin=Home_Z_Pin;

  B.step_pin=Step_B_Pin;
  B.dir_pin=Dir_B_Pin;
  B.home_pin=Home_B_Pin;

  C.step_pin=Step_C_Pin;
  C.dir_pin=Dir_C_Pin;
  C.home_pin=Home_C_Pin;


  uint32_t contador=0;
  printf("Test_commit sin_PPS4");
  printf("Test_commit sin_PPS5");
  while (1)
  {

	  /*contador++;
	  if(contador%300000==0){
		  printf("EXYZBC\n");
		  printf("%i%i%i%i%i%i\n",HAL_GPIO_ReadPin(GPIOA, E_Stop_Pin),HAL_GPIO_ReadPin(GPIOA, Home_X_Pin),HAL_GPIO_ReadPin(GPIOA, Home_Y_Pin),HAL_GPIO_ReadPin(GPIOA, Home_Z_Pin),HAL_GPIO_ReadPin(GPIOA, Home_B_Pin),HAL_GPIO_ReadPin(GPIOA, Home_C_Pin));

	  }*/
	  if(GPIOA -> IDR & E_Stop_Pin)	parada_de_emergencia=1;
	  else	  parada_de_emergencia=0;

	  if(N_linea_actual!=indiceBuferLINEA){
		  //uint8_t i=0;
		  //while(bufferUART[i][N_linea_actual]!=0) {
			//  printf("%c",bufferUART[i][N_linea_actual]);
			//  i++;
		  //}
		  //printf("\n");
		  analizarLineaUART(N_linea_actual);
		  N_linea_actual++;
		  if(N_linea_actual>maxima_cant_lineas)	N_linea_actual=0;
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Step_X_Pin|Dir_X_Pin|Step_Y_Pin|Dir_Y_Pin
                          |Step_Z_Pin|Dir_Z_Pin|Step_C_Pin|Dir_C_Pin
                          |Step_B_Pin|Dir_B_Pin|Enable_motors_Pin|Vacuum_out_Pin
                          |GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : Home_C_Pin Home_B_Pin Home_Z_Pin Home_Y_Pin
                           Home_X_Pin E_Stop_Pin */
  GPIO_InitStruct.Pin = Home_C_Pin|Home_B_Pin|Home_Z_Pin|Home_Y_Pin
                          |Home_X_Pin|E_Stop_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Step_X_Pin Dir_X_Pin Step_Y_Pin Dir_Y_Pin
                           Step_Z_Pin Dir_Z_Pin Step_C_Pin Dir_C_Pin
                           Step_B_Pin Dir_B_Pin Enable_motors_Pin Vacuum_out_Pin
                           PB8 PB9 */
  GPIO_InitStruct.Pin = Step_X_Pin|Dir_X_Pin|Step_Y_Pin|Dir_Y_Pin
                          |Step_Z_Pin|Dir_Z_Pin|Step_C_Pin|Dir_C_Pin
                          |Step_B_Pin|Dir_B_Pin|Enable_motors_Pin|Vacuum_out_Pin
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Sensing_vac_in_Pin */
  GPIO_InitStruct.Pin = Sensing_vac_in_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Sensing_vac_in_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */


uint16_t index_linea_UART=0;
uint32_t mensaje_linea_UART=0;
double G_value=0,P_value=0,S_value=0,F_value=0,M_value=0,X_value=0,Y_value=0,Z_value=0,B_value=0,C_value=0;
#define double_max_value 100000
uint8_t modo_coord_absolutas=1;
void analizarLineaUART(uint8_t N_linea){
	if(bufferUART[0][N_linea]>='A'&&bufferUART[0][N_linea]<='Z'){//linea G code
		M_value=-1;
		X_value=double_max_value;
		Y_value=double_max_value;
		Z_value=double_max_value;
		B_value=double_max_value;
		C_value=double_max_value;
		uint8_t i=0;
		while(bufferUART[i][N_linea]!=0||i>tamano_buffer_UART-2){
			if(bufferUART[i][N_linea]=='G') G_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='P') P_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='S') S_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='F') F_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='M') M_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='X') X_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='Y') Y_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='Z') Z_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='B') B_value=string_to_double(&i,N_linea);
			if(bufferUART[i][N_linea]=='C') C_value=string_to_double(&i,N_linea);
			//printf("valores: ");
			//print_double(X_value);
			//printf("\n");
			i++;
		}

		if(M_value!=-1){
			if(M_value==90){
				modo_coord_absolutas=1;//coordenadas absolutas
				print_G0_fin_and_coordenadas();
			}
			if(M_value==91){
				modo_coord_absolutas=0;//coordenadas relativas
				print_G0_fin_and_coordenadas();
			}
			if(M_value==17) {
				HAL_GPIO_WritePin(Enable_motors_GPIO_Port, Enable_motors_Pin, 1);//apagar motores
				print_G0_fin_and_coordenadas();
			}
			if(M_value==18) {
				HAL_GPIO_WritePin(Enable_motors_GPIO_Port, Enable_motors_Pin, 0);//encender
				analizar_switch_arranque();
				home_inicio_sec(home_inicio);
				print_G0_fin_and_coordenadas();
			}
			if(M_value==9) HAL_GPIO_WritePin(Vacuum_out_GPIO_Port, Vacuum_out_Pin, 0);//apagar valvula
			if(M_value==7) HAL_GPIO_WritePin(Vacuum_out_GPIO_Port, Vacuum_out_Pin, 1);//encender valvula
			if(M_value==114) {
				print_G0_fin_and_coordenadas();
			}
			if(M_value==150) print_G0_fin_and_coordenadas();
			if(M_value==151) print_G0_fin_and_coordenadas();
			//if(M_value==150){ //comando luces M150 Si Pv; i=numero de lampara, v=valor PWM (0-255)//OBSOLETO
			//	if(S_value!=-1&&P_value!=-1){
			//		if(S_value==0) TIM3->CCR1 = ((uint16_t)P_value)<<8;//lampara cero
			//		if(S_value==1) TIM3->CCR2 = ((uint16_t)P_value)<<8;//lampara uno
			//		S_value=-1;
			//		P_value=-1;
			//	}
			//}
			M_value=-1;
		}
		if(G_value==90){//coordenadas absolutas
			modo_coord_absolutas=1;
			print_G0_fin_and_coordenadas();
			G_value=-1;
		}
		if(G_value==91){//coordenadas relativas
			modo_coord_absolutas=0;
			print_G0_fin_and_coordenadas();
			G_value=-1;
		}
		if(G_value==92){//set posicion (global offset)
			if(X_value!=double_max_value) X.step_mm_actual=X_value*X.step_mm;
			if(X_value!=double_max_value) X.step_mm_goto=X_value*X.step_mm;

			if(Y_value!=double_max_value) Y.step_mm_actual=Y_value*Y.step_mm;
			if(Y_value!=double_max_value) Y.step_mm_goto=Y_value*Y.step_mm;

			if(Z_value!=double_max_value) Z.step_mm_actual=Z_value*Z.step_mm;
			if(Z_value!=double_max_value) Z.step_mm_goto=Z_value*Z.step_mm;

			if(B_value!=double_max_value) B.step_mm_actual=B_value*B.step_mm;
			if(B_value!=double_max_value) B.step_mm_goto=B_value*B.step_mm;

			if(C_value!=double_max_value) C.step_mm_actual=C_value*C.step_mm;
			if(C_value!=double_max_value) C.step_mm_goto=C_value*C.step_mm;

			G_value=-1;
			print_G0_fin_and_coordenadas();
		}
		if(G_value==0||G_value==1){
			if(modo_coord_absolutas==0){//relativas
				if(X_value!=double_max_value) X.step_mm_goto+=X_value*X.step_mm;
				if(Y_value!=double_max_value) Y.step_mm_goto+=Y_value*Y.step_mm;
				if(Z_value!=double_max_value) Z.step_mm_goto+=Z_value*Z.step_mm;
				if(B_value!=double_max_value) B.step_mm_goto+=B_value*B.step_mm;
				if(C_value!=double_max_value) C.step_mm_goto+=C_value*C.step_mm;
			}else{//absolutas
				if(X_value!=double_max_value)X.step_mm_goto=X_value*X.step_mm;
				if(Y_value!=double_max_value)Y.step_mm_goto=Y_value*Y.step_mm;
				if(Z_value!=double_max_value)Z.step_mm_goto=Z_value*Z.step_mm;
				if(B_value!=double_max_value)B.step_mm_goto=B_value*B.step_mm;
				if(C_value!=double_max_value)C.step_mm_goto=C_value*C.step_mm;
			}
			run_motores(G_value,F_value);
			if(!parada_de_emergencia) {
				print_G0_fin_and_coordenadas();
				print_G0_fin_and_coordenadas();
			}
		}
		if(G_value==28){//homing
			home_secuencia();
			printf("G28_fin\n");
			print_G0_fin_and_coordenadas();
			G_value=-1;
		}
	}
	if(bufferUART[0][N_linea]==':'){//con ":" se configuran los mensajes
		index_linea_UART=_HEX_to_int(bufferUART[1][N_linea])<<12;//no lo hago con un loop para mayor velocidad
		index_linea_UART+=_HEX_to_int(bufferUART[2][N_linea])<<8;
		index_linea_UART+=_HEX_to_int(bufferUART[3][N_linea])<<4;
		index_linea_UART+=_HEX_to_int(bufferUART[4][N_linea]);

		mensaje_linea_UART=_HEX_to_int(bufferUART[5][N_linea])<<28;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[6][N_linea])<<24;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[7][N_linea])<<20;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[8][N_linea])<<16;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[9][N_linea])<<12;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[10][N_linea])<<8;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[11][N_linea])<<4;
		mensaje_linea_UART+=_HEX_to_int(bufferUART[12][N_linea]);
		if(index_linea_UART>=0x10&&index_linea_UART<tamano_vector)	vector_datos_interfaz[index_linea_UART]=mensaje_linea_UART;

		if(index_linea_UART==0x0);//	0	32bits	retorna en el index 0x0 el valor recibido
		if(index_linea_UART==0x1){//	1	nada	seteas las variables en el micro con los valores del vector recibido a patir del index 0x10 y guarda en la flash
			vector_to_Flash();
			vector_to_RAM();
		}
		if(index_linea_UART==0x2){//	2	nada	devuelve el vector completo a patrir del index 0x10
			print_vecotr();
		}
		if(index_linea_UART==0x3){//	3	nada	retorna en el index 0x3 el acsi pasado a HEX de la version "1.0" (maximo cuatro caracteres)
			printf(":%04X%08X\n",0x03,(uint16_t)(version_float*100));
		}
		if(index_linea_UART==0x4){//	4	nada	setea el vector sin guardar en la flash
			vector_to_RAM();
		}
		printf("index: %i , value: %i\n",index_linea_UART,mensaje_linea_UART);
	}
	for(uint16_t i=0;i<tamano_buffer_UART;i++) bufferUART[i][N_linea]=0;
}
void print_G0_fin_and_coordenadas(){
if(modo_coord_absolutas==0) printf("RelX");
else printf("AbsX");
print_double(((double)X.step_mm_actual)/X.step_mm);
printf(" Y");
print_double(((double)Y.step_mm_actual)/Y.step_mm);
printf(" Z");
print_double(((double)Z.step_mm_actual)/Z.step_mm);
printf(" C");
print_double(((double)C.step_mm_actual)/C.step_mm);
printf(" B");
print_double(((double)B.step_mm_actual)/B.step_mm);
printf("\n");
printf("G0_fin\n");
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)//por cada caracter que llegue entra a esta interrupcion, el dato esta en auxData bla bla bla
{
	if(auxCaracterInSerie=='\n'){
		indiceBuferUART=0;
		indiceBuferLINEA++;
		if(indiceBuferLINEA>maxima_cant_lineas) indiceBuferLINEA=0;
	}else{
		if(indiceBuferUART<tamano_buffer_UART-1){
			bufferUART[indiceBuferUART][indiceBuferLINEA]=auxCaracterInSerie;
			indiceBuferUART++;
			bufferUART[indiceBuferUART][indiceBuferLINEA]=0;
		}
	}
	HAL_UART_Receive_IT(&huart1, &auxCaracterInSerie, 1);
}

void vector_to_RAM(){
	X.v_max= vector_datos_interfaz[0x10];
	Y.v_max= vector_datos_interfaz[0x11];
	Z.v_max= vector_datos_interfaz[0x12];
	B.v_max= vector_datos_interfaz[0x13];
	C.v_max= vector_datos_interfaz[0x14];
	X.acc=vector_datos_interfaz[0x15];
	Y.acc=vector_datos_interfaz[0x16];
	Z.acc=vector_datos_interfaz[0x17];
	B.acc=vector_datos_interfaz[0x18];
	C.acc=vector_datos_interfaz[0x19];
	X.step_mm=((double)vector_datos_interfaz[0x1A])/1000000;
	Y.step_mm=((double)vector_datos_interfaz[0x1B])/1000000;
	Z.step_mm=((double)vector_datos_interfaz[0x1C])/1000000;
	B.step_mm=((double)vector_datos_interfaz[0x1D])/1000000;
	C.step_mm=((double)vector_datos_interfaz[0x1E])/1000000;
	X.porcentaje_v_min=vector_datos_interfaz[0x1F];
	Y.porcentaje_v_min=vector_datos_interfaz[0x20];
	Z.porcentaje_v_min=vector_datos_interfaz[0x21];
	B.porcentaje_v_min=vector_datos_interfaz[0x22];
	C.porcentaje_v_min=vector_datos_interfaz[0x23];
	X.invertir_direccion=0b1&(vector_datos_interfaz[0x2D]>>0);
	Y.invertir_direccion=0b1&(vector_datos_interfaz[0x2D]>>1);
	Z.invertir_direccion=0b1&(vector_datos_interfaz[0x2D]>>2);
	B.invertir_direccion=0b1&(vector_datos_interfaz[0x2D]>>3);
	C.invertir_direccion=0b1&(vector_datos_interfaz[0x2D]>>4);

	X.home_act=0b1&(vector_datos_interfaz[0x2E]>>0);
	Y.home_act=0b1&(vector_datos_interfaz[0x2E]>>1);
	Z.home_act=0b1&(vector_datos_interfaz[0x2E]>>2);
	B.home_act=0b1&(vector_datos_interfaz[0x2E]>>3);
	C.home_act=0b1&(vector_datos_interfaz[0x2E]>>4);
	X.home_dir=0b1&(vector_datos_interfaz[0x2F]>>0);
	Y.home_dir=0b1&(vector_datos_interfaz[0x2F]>>1);
	Z.home_dir=0b1&(vector_datos_interfaz[0x2F]>>2);
	B.home_dir=0b1&(vector_datos_interfaz[0x2F]>>3);
	C.home_dir=0b1&(vector_datos_interfaz[0x2F]>>4);
	X.home_sw_nromal_0_1=0b1&(vector_datos_interfaz[0x30]>>0);
	Y.home_sw_nromal_0_1=0b1&(vector_datos_interfaz[0x30]>>1);
	Z.home_sw_nromal_0_1=0b1&(vector_datos_interfaz[0x30]>>2);
	B.home_sw_nromal_0_1=0b1&(vector_datos_interfaz[0x30]>>3);
	C.home_sw_nromal_0_1=0b1&(vector_datos_interfaz[0x30]>>4);
	X.home_porcentaje_vel=vector_datos_interfaz[0x31];
	Y.home_porcentaje_vel=vector_datos_interfaz[0x32];
	Z.home_porcentaje_vel=vector_datos_interfaz[0x33];
	B.home_porcentaje_vel=vector_datos_interfaz[0x34];
	C.home_porcentaje_vel=vector_datos_interfaz[0x35];
	X.home_mm_retroceso=vector_datos_interfaz[0x36];
	Y.home_mm_retroceso=vector_datos_interfaz[0x37];
	Z.home_mm_retroceso=vector_datos_interfaz[0x38];
	B.home_mm_retroceso=vector_datos_interfaz[0x39];
	C.home_mm_retroceso=vector_datos_interfaz[0x3A];
	X.home_ubic_mm=((double)vector_datos_interfaz[0x3B]-200000000)/1000000;
	Y.home_ubic_mm=((double)vector_datos_interfaz[0x3C]-200000000)/1000000;
	Z.home_ubic_mm=((double)vector_datos_interfaz[0x3D]-200000000)/1000000;
	B.home_ubic_mm=((double)vector_datos_interfaz[0x3E]-200000000)/1000000;
	C.home_ubic_mm=((double)vector_datos_interfaz[0x3F]-200000000)/1000000;
	X.home_prioridad=0b1111&(vector_datos_interfaz[0x40]>>0);
	Y.home_prioridad=0b1111&(vector_datos_interfaz[0x40]>>4);
	Z.home_prioridad=0b1111&(vector_datos_interfaz[0x40]>>8);
	B.home_prioridad=0b1111&(vector_datos_interfaz[0x40]>>12);
	C.home_prioridad=0b1111&(vector_datos_interfaz[0x40]>>16);

	home_inicio=vector_datos_interfaz[0x41];



	printf("Ram seteada\n");
}
void vector_to_Flash(){
	uint32_t ErrorPagina;
	HAL_StatusTypeDef status;
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef eraseStruct;
	eraseStruct.TypeErase=FLASH_TYPEERASE_PAGES;
	eraseStruct.PageAddress=inicioPagina_flash;
	eraseStruct.NbPages=3;
	HAL_FLASHEx_Erase(&eraseStruct, &ErrorPagina);
	for(uint8_t i=0x10;i<tamano_vector;i++) HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, inicioPagina_flash+i*4,vector_datos_interfaz[i]);  //Write
	HAL_FLASH_Lock();
	printf("flash seteada\n");
}
void Flash_to_vector(){
	for(uint8_t i=0x10;i<tamano_vector;i++) vector_datos_interfaz[i]=*(uint32_t*)(inicioPagina_flash+i*4);//Read
}
void print_vecotr(){
	for(uint8_t i=0x10;i<tamano_vector;i++) {
		printf(":%04X%08X\n",i,vector_datos_interfaz[i]);
		for(uint32_t x=0;x<90000;x++) asm("NOP");//delay
	}
	printf(":%04X%08X\n",0x0A,0);
}
void print_double(double val){
	if(val<0) {
		val=-val;
		printf("-");
	}

	printf("%04i.%06i",(int)val,(int)((val-(int)val)*1000000));
	//else if(val==0) printf("%04i.%06i",(int)val,(int)((val-(int)val)*1000000));

}
double string_to_double(uint8_t* index,uint8_t N_linea){
	double ret=0;
	double decimal=0;
	*index=*index+1;
	uint8_t negativo=0;
	if(bufferUART[*index][N_linea]=='-') {
		negativo=1;
		*index=*index+1;
	}
	//while(bufferUART[*index][N_linea]>32){
	while(bufferUART[*index][N_linea]>=','&&bufferUART[*index][N_linea]<='9'){
		if(bufferUART[*index][N_linea]=='.'||bufferUART[*index][N_linea]==',') {
			decimal=1;
			*index=*index+1;
		}
		ret*=10;
		ret+=bufferUART[*index][N_linea]-48;
		decimal*=10;
		*index=*index+1;
	}
	if(decimal==0) decimal=1;
	if(negativo) 	return -ret/decimal;
	else 			return ret/decimal;
}
uint8_t _HEX_to_int(char c){
	if(c>=48&&c<=57) return c-48;
	if(c>=65&&c<=70) return c-55;
	if(c>=97&&c<=102) return c-87;
	return 0;
}
void analizar_switch_arranque(){
	if(GPIOA -> IDR & E_Stop_Pin){//switch pulsado en el arranque (aca solo busca despulsarlo en la direccion de busqueda de home)
		  despulsar_limite_arranque(X);
		  despulsar_limite_arranque(Y);
		  despulsar_limite_arranque(Z);
		  despulsar_limite_arranque(B);
		  despulsar_limite_arranque(C);
	  }
}
void despulsar_limite_arranque(struct eje_str eje){
	HAL_GPIO_WritePin(GPIOB, eje.dir_pin, 1-eje.home_dir);
	if(eje.invertir_direccion) HAL_GPIO_TogglePin(GPIOB, eje.dir_pin);
	HAL_GPIO_TogglePin(GPIOB, eje.dir_pin);//toggleo una vez mas porque despues la funcion despulsar_eje hace un toggle
	if(eje.home_act)  despulsar_eje(eje);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
