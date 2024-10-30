/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "seven_seg.h"


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define BIT0 	0b00000001
#define BIT1 	0b00000010
#define BIT2 	0b00000100
#define BIT3 	0b00001000
#define BIT4 	0b00010000
#define BIT5 	0b00100000
#define BIT6 	0b01000000
#define BIT7 	0b10000000

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RCK_Pin GPIO_PIN_4
#define RCK_GPIO_Port GPIOA
#define COILA_TX_Pin GPIO_PIN_6
#define COILA_TX_GPIO_Port GPIOB
#define COILB_TX_Pin GPIO_PIN_7
#define COILB_TX_GPIO_Port GPIOB
#define COILA_RX_Pin GPIO_PIN_8
#define COILA_RX_GPIO_Port GPIOB
#define COILB_RX_Pin GPIO_PIN_9
#define COILB_RX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// Soft UART numbers
#define SOFT_UART_A	0
#define SOFT_UART_B 1

// Communication standard
#define SENSE_BOARD_MSG_SIZE	5

#define MSG_TEMP_INT      0
#define MSG_TEMP_FLOAT    1
#define MSG_VOLTAGE_INT   2
#define MSG_VOLTAGE_FLOAT 3
#define MSG_CURRENT_INT   4
#define MSG_CURRENT_FLOAT 5


#define boolean uint8_t
#define true  1
#define false 0


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
