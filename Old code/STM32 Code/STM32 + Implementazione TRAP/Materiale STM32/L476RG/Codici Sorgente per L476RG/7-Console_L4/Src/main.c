/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define Vref		3300.0f
#define Avg_Slope	2.50f
#define V25			760.0f

#define TS110 	((uint16_t*)((uint32_t) 0x1FFF75CA))// calibrated at 3.3V +-10mV @110C +/- 5C
#define TS30 	((uint16_t*)((uint32_t) 0x1FFF75A8))// calibrated at 3.3V +-10mV @ 30C +/- 5C
#define VREF    ((uint16_t*)((uint32_t) 0x1FFF75AA))// calibrated at 3.3V +-10mV @ 30C +/- 5C

#define MAXBUFFSIZE    	100
#define CMD_NUM 		8

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

volatile uint16_t press_number = 0;

uint8_t idx = 0;
uint16_t periods[4] = {249, 499, 999, 1999};
uint8_t pulse[4] = {1, 4, 9, 0};

uint8_t dc, step = 0;

uint8_t flag = 1;

uint8_t buffer[MAXBUFFSIZE];

uint8_t RxBuf [MAXBUFFSIZE];
uint8_t TxBuf [MAXBUFFSIZE];

uint8_t errorBuff[] = "\n\r*** Command not found ****\n\r";
uint8_t welcomeBuff[] = "**** WELCOME TO STM32 CONSOLE ****\n\r";


uint8_t *cmds[CMD_NUM] = {
		"led on",
		"led off",
		"led toggle",
		"led blink",
		"led status",
		"get timer",
		"get temp",
		"set timer"
};

uint8_t char_counter = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void pwm_dc(uint16_t val);
uint16_t get_ADC();
float ConvertTemp(uint16_t adc_raw);
void cmd_check();
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
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Transmit(&huart2, welcomeBuff, sizeof(welcomeBuff), 5000);
  HAL_UART_Receive_IT(&huart2 , RxBuf, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 15;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the ADC multi-mode 
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 59999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 59999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 499;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_13){						//Check if the interrupt comes from the right GPIO

/*************** EX 1 ***************/
//
			//HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); //LED TOGGLING
//
//			press_number++; 							// Increment the number of time we press the button
//		}
//
/************************************/

/*************** EX 2 ***************/
//
//		if(press_number++ % 2 == 0)						// Check if the number is even/odd
//			HAL_TIM_Base_Start_IT(&htim10);
//
//		else
//			HAL_TIM_Base_Stop_IT(&htim10);
//
//
/************************************/

/*************** EX 3 ***************/
//
//		if(idx < 4){									// Check the index used to switch timer period
//
//			HAL_TIM_Base_Stop_IT(&htim10);				// Stop the timer
//
//			TIM10->ARR = periods[idx];					// Update the auto reload register
//			TIM10->CNT = 0;								// Clear the count register
//
//			HAL_TIM_Base_Start_IT(&htim10);				// Start the timer
//
//			idx++;										// Increment index
//		} else{
//
//			HAL_TIM_Base_Stop_IT(&htim10);				// Stop the timer
//
//			idx = 0;									// Clear Index
//
//		}
//
/************************************/

/*************** EX 4 ***************/

//		if(idx < 4){
//
//			HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
//
//			TIM2->CCR1 = pulse [idx];
//
//			HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
//
//			idx++;
//
//		}else {
//
//			idx = 0;
//
//		}

/************************************/

/*************** EX 6 ***************/
//
//		if(press_number++ % 2 == 0){						// Check if the number is even/odd
//			HAL_TIM_Base_Start_IT(&htim6);
//			dc = 0;
//		}
//		else{
//			HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
//			HAL_TIM_Base_Stop_IT(&htim6);
//			dc = 0;
//		}
//
/************************************/

/*************** EX 7 ***************/

//		if(flag){
//
//			HAL_UART_Transmit(&huart2, buffer, sizeof(buffer), 5000);
//			flag = 0;
//			HAL_TIM_Base_Start_IT(&htim6);
//
//		}

/************************************/
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if(htim == &htim6){								// Check if the interrupt comes from the right Timer
//		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); 			//LED TOGGLING

/*************** EX 6 ***************/
//
//		if (dc == 0) step = 1;							// Check if we want to increment the duty cycle
//		if (dc == 10) step = -1;						// Check if we want to decrement the duty cycle
//
//		dc += step;										//Duty cycle increment/decrement
//		pwm_dc(dc);										//Duty cycle update
//	}
//
/************************************/

/*************** EX 7 ***************/

//		flag = 1;
//		HAL_TIM_Base_Stop_IT(&htim6);

/************************************/

/*************** EX 8 ***************/
//		sprintf(buffer, "Temp: %f \n\r", ConvertTemp(get_ADC()));
//
//		HAL_UART_Transmit(&huart2, buffer, sizeof(buffer), 5000);
/************************************/

/*************** EX 9 ***************/
//
//		double t = ConvertTemp(get_ADC());
//		//double t = get_ADC();
//
//		if(t < 23)
//			pwm_dc(3);
//
//		else if( t >= 23 && t <= 25)
//			pwm_dc(4);
//
//		else
//			pwm_dc(9);
//
//		sprintf((char*)buffer, "Temp: %4.2lf \n\r", t);
//
//		HAL_UART_Transmit(&huart2, buffer, sizeof(buffer), 5000);
//
/************************************/

/*************** EX 10 ***************/

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

/************************************/
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	if(RxBuf[0] == '\r'){

		HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", sizeof("\r\n"),10);
	    cmd_check();
	}
	else if(RxBuf[0] == '\b'){

		HAL_UART_Transmit(&huart2, (uint8_t *)"\b \b", sizeof("\b \b"),10);
		TxBuf[--char_counter] = NULL;

	}else{

		TxBuf[char_counter++] = RxBuf[0];

		HAL_UART_Transmit(&huart2, (uint8_t *) RxBuf, sizeof(RxBuf),10);
	}

	HAL_UART_Receive_IT(&huart2 , RxBuf, 1); // Restart UART in Interrupt mode
}

void pwm_dc(uint16_t val){

	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

	TIM2->CCR1 = val;

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

}

float ConvertTemp(uint16_t D_ADC){

	float V_ADC = (D_ADC * ( Vref / 4095.0 ));			// Convert bit to voltage
	float processorTemp;                    			// the temperature in degree Celsius

	processorTemp =  V_ADC - V25;
	processorTemp /= Avg_Slope ;
	processorTemp += 45;								//Temperature offset
														//to be calibrated
	return processorTemp;
}

uint16_t get_ADC(){

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	uint16_t value = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	return value;
}

void cmd_check(){

	int8_t i = CMD_NUM - 1;

//	HAL_UART_Transmit(&huart2, (uint8_t *)"\n\r***\n\r", sizeof("\n\r***\n\r"),1000);
//	HAL_UART_Transmit(&huart2, (uint8_t *) TxBuf, sizeof(TxBuf),1000);
//	HAL_UART_Transmit(&huart2, (uint8_t *)"\n\r***\n\r", sizeof("\n\r***\n\r"),1000);

	while(strcmp((char *)TxBuf, (char *)cmds[i]) != 0 && i >= 0)
		i--;

	if(i < 0){
		memset(buffer, 0, MAXBUFFSIZE);
		sprintf((char*)buffer, "Unrecognized command!\n\rCommand list: \r\n");
		HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);

		for (uint8_t i = 0; i < CMD_NUM; i++){

			memset(buffer, 0, MAXBUFFSIZE);
			sprintf((char*)buffer, "%i - %s\r\n", i, cmds[i]);
			HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);
		}
	}

	else{

		memset(buffer, 0, MAXBUFFSIZE);
		sprintf((char*)buffer, "Command OK. ID: %d\r\n",i);
		HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);

		switch(i){

			case 0:
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, SET);
				break;

			case 1:
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET);
				HAL_TIM_Base_Stop_IT(&htim6);
				TIM6->CNT = 0;
				break;

			case 2:
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
				break;

			case 3:
				//HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
				HAL_TIM_Base_Start_IT(&htim6);
				break;

			case 4:
				if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)){

					memset(buffer, 0, MAXBUFFSIZE);
					sprintf((char*)buffer, "Led ON\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);

				}
				else{

					memset(buffer, 0, MAXBUFFSIZE);
					sprintf((char*)buffer, "Led OFF\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);

				}
				break;

			case 5:
				if(TIM6->CNT == 0){
					memset(buffer, 0, MAXBUFFSIZE);
					sprintf((char*)buffer, "Timer 6 is stoped!\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);
				} else {

					memset(buffer, 0, MAXBUFFSIZE);
					sprintf((char*)buffer, "Timer 6 is running\r\n");
					HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);
				}
				break;

			case 6: ;
				memset(buffer, 0, MAXBUFFSIZE);
				sprintf((char*)buffer, "Temp: %f \r\n", ConvertTemp( get_ADC()));
				HAL_UART_Transmit(&huart2, (uint8_t *) buffer, sizeof(buffer),1000);
				break;
		}
	}

	memset(TxBuf, 0, MAXBUFFSIZE);
	memset(RxBuf, 0, MAXBUFFSIZE);
	char_counter = 0;
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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
