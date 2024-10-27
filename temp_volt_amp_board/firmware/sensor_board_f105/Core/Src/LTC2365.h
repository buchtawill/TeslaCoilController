#ifndef LTC2365_H
#define LTC2365_H

#include "stm32f1xx_hal.h"  // STM32 HAL library for GPIO and SPI functions

#define ADC1_CS_PORT GPIOA
#define ADC1_CS_PIN  GPIO_PIN_4  // Adjust as per your schematic

#define ADC2_CS_PORT GPIOB
#define ADC2_CS_PIN  GPIO_PIN_12  // Adjust as per your schematic

uint16_t Read_ADC1_Averaged(void);

#endif // LTC2365
