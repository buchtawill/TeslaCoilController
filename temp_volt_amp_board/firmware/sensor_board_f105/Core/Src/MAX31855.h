#ifndef MAX31855_H
#define MAX31855_H

#include "stm32f1xx_hal.h"  // STM32 HAL library for GPIO and SPI functions

// Define the GPIO port and pin for Chip Select
#define MAX31855_CS_PORT GPIOA
#define MAX31855_CS_PIN GPIO_PIN_15
#define NUM_SAMPLES 30

// Function declarations
void MAX31855_Init(void);
uint32_t MAX31855_ReadRaw(void);
float MAX31855_GetTemperature(void);

#endif // MAX31855_H
