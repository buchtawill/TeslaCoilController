/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void changeNumber(uint16_t* number, uint16_t max, uint8_t printPosition);
void delayMicroseconds(uint64_t delay);
void SDMode();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void delayMicroseconds(uint64_t delay);
void writeStatusLED(uint8_t status);
void board_ok();
void sendUSB(char *s, uint16_t bytes);
uint64_t getMicros();
void USBDataReceived_IT(uint8_t* Buf, uint32_t *len);
void printToUSB(char* s);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define stat595_SER_Pin GPIO_PIN_13
#define stat595_SER_GPIO_Port GPIOC
#define Pot1_Pin GPIO_PIN_0
#define Pot1_GPIO_Port GPIOA
#define Pot2_Pin GPIO_PIN_2
#define Pot2_GPIO_Port GPIOA
#define stat595_SCK_Pin GPIO_PIN_4
#define stat595_SCK_GPIO_Port GPIOA
#define LED_Heartbeat_Pin GPIO_PIN_0
#define LED_Heartbeat_GPIO_Port GPIOB
#define Flash__wp_Pin GPIO_PIN_1
#define Flash__wp_GPIO_Port GPIOB
#define Flash__Hold_Pin GPIO_PIN_2
#define Flash__Hold_GPIO_Port GPIOB
#define SPI2_SS_Pin GPIO_PIN_12
#define SPI2_SS_GPIO_Port GPIOB
#define ROT_BUT_Pin GPIO_PIN_8
#define ROT_BUT_GPIO_Port GPIOA
#define ROT_BUT_EXTI_IRQn EXTI9_5_IRQn
#define ROT_CLK_Pin GPIO_PIN_9
#define ROT_CLK_GPIO_Port GPIOA
#define ROT_CLK_EXTI_IRQn EXTI9_5_IRQn
#define ROT_DAT_Pin GPIO_PIN_10
#define ROT_DAT_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_15
#define SD_CS_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */
#define boolean uint8_t
#define true  	1
#define false 	0
#define COIL1 	htim2
#define COIL2 	htim9
#define COIL3 	htim3
#define COIL4 	htim4
#define COIL5 	htim10

#define COIL1_CH TIM_CHANNEL_1
#define COIL2_CH TIM_CHANNEL_2
#define COIL3_CH TIM_CHANNEL_1
#define COIL4_CH TIM_CHANNEL_1
#define COIL5_CH TIM_CHANNEL_1



#define SD_SPI_HANDLE 	hspi1
//#define SD_CS_GPIO_Port	SPI1_SS_GPIO_Port
//#define SD_CS_Pin		SPI1_SS_Pin

/* State Definition ---------------------------------------------------------*/
#define MODE_SELECT 0 //SD Card, Burst, or Fixed
#define SD_MODE 1
#define BURST_MODE 2
#define FIXED_MODE 3

#define MODE_NUM 3

//State within Burst mode
#define B_FREQUENCY 	0
#define B_TON 			1
#define B_TOFF 			2
#define B_PLAY_PAUSE 	3
#define B_BACK 			4
#define BURST_STATE_NUM 4

//State within Fixed mode
#define F_FREQUENCY 	0
#define F_PLAY_PAUSE 	1
#define F_BACK 			2
#define FIXED_STATE_NUM 2

// Change configuration with number state
#define HUNDRED_DIGIT 0
#define TEN_DIGIT 1
#define SINGLE_DIGIT 2


/* Display constant Definition ------------------------------------------------------*/
#define MAX_ROW 			4
#define MAX_CHAR_ON_SCREEN 	19 // Maximum number of characters the display can show
#define MAX_NUM_FILES    	32
#define MAX_FILENAME_LENGTH 64

#define FREQ_DISP_POS 	6
#define T_ON_DISP_POS 	6
#define T_OFF_DISP_POS 	7
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
