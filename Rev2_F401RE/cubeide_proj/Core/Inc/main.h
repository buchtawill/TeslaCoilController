/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ROT_CLK_Pin GPIO_PIN_13
#define ROT_CLK_GPIO_Port GPIOC
#define ROT_CLK_EXTI_IRQn EXTI15_10_IRQn
#define ROT_DAT_Pin GPIO_PIN_14
#define ROT_DAT_GPIO_Port GPIOC
#define ROT_SW_Pin GPIO_PIN_15
#define ROT_SW_GPIO_Port GPIOC
#define ROT_SW_EXTI_IRQn EXTI15_10_IRQn
#define OH_SHIT_BTN_Pin GPIO_PIN_0
#define OH_SHIT_BTN_GPIO_Port GPIOC
#define OH_SHIT_BTN_EXTI_IRQn EXTI0_IRQn
#define OLED_CS_Pin GPIO_PIN_4
#define OLED_CS_GPIO_Port GPIOA
#define OLED_RST_Pin GPIO_PIN_4
#define OLED_RST_GPIO_Port GPIOC
#define OLED_DC_Pin GPIO_PIN_5
#define OLED_DC_GPIO_Port GPIOC
#define SPKR_EN_BTN_Pin GPIO_PIN_1
#define SPKR_EN_BTN_GPIO_Port GPIOB
#define SPKR_EN_BTN_EXTI_IRQn EXTI1_IRQn
#define SPKR_EN_Pin GPIO_PIN_12
#define SPKR_EN_GPIO_Port GPIOB
#define LED_HEARTBEAT_Pin GPIO_PIN_13
#define LED_HEARTBEAT_GPIO_Port GPIOB
#define SDIO_DETECT_Pin GPIO_PIN_14
#define SDIO_DETECT_GPIO_Port GPIOB
#define RELAY_Pin GPIO_PIN_15
#define RELAY_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

typedef enum{
	S_CHOOSE_MODE = 0,
	S_CHOOSE_SD_SONG,
	S_PLAYING_KEYBOARD,
	S_PLAYING_SD_SONG,
	S_PRE_COOLDOWN,
	S_COOLDOWN
} UCState;

//State definitions
#define INTERRUPTER_MODE_CHOOSE   0 //Choose the interrupter mode
#define INTERRUPTER_MODE_SD       1 //SD mode
#define INTERRUPTER_MODE_LIVE     2 //Live mode
#define INTERRUPTER_MODE_BURST    3 //Burst mode
#define INTERRUPTER_MODE_FIXED    4 //Fixed mode
#define INTERRUPTER_NUM_MODES     5

//Burst mode sub-state
#define BURST_CHANGE_FREQ         0
#define BURST_CHANGE_TON          1
#define BURST_CHANGE_TOFF         2
#define BURST_PLAY_PAUSE          3
#define BURST_GO_BACK             4
#define BURST_NUM_STATES          5

//Fixed mode sub-state
#define FIXED_CHANGE_FREQ         0
#define FIXED_PLAY_PAUSE          1
#define FIXED_GO_BACK             2
#define FIXED_NUM_STATES          3

#define HUNDRED_DIGIT             0
#define TEN_DIGIT                 1
#define ONES_DIGIT                2

#define MAX_ROW_LCD  		4
#define MAX_CHAR_ON_SCREEN 	19 // Maximum number of characters the display can show
#define MAX_NUM_FILES    	32
#define MAX_FILENAME_LENGTH 64

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef  htim11;

extern I2C_HandleTypeDef hi2c2;

#define KEYBOARD_UART_HANDLE huart1
#define MILLIS_TIMER_HANDLE  htim11
#define SD_TIMER_HANDLE htim9

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
