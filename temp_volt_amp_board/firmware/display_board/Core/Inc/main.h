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

#define DIGITS(i) ((BIT4 | BIT0) << i)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RCK_Pin GPIO_PIN_4
#define RCK_GPIO_Port GPIOA
#define OE_B_Pin GPIO_PIN_15
#define OE_B_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/**
 * Outgoing SPI message struct containing data for 7 shift registers.
 * current[0] is the display on the left, current[1] is the display on the right.
 * Same is true for voltage and temp.
 * digits controls which digits are to be on.
 *
 * Set digits to be one of DIGITS(i), where 0 is the left digit, 3 is the rightmost digit
 */
typedef struct outgoing_msg{
	uint8_t current[2];
	uint8_t voltage[2];
	uint8_t temp[2];
	uint8_t digits;
}OutgoingMessage;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
