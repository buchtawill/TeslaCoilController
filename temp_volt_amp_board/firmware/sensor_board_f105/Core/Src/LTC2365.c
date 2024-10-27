#include "LTC2365.h"

extern SPI_HandleTypeDef hspi1; // SPI1 handle
extern SPI_HandleTypeDef hspi2; // SPI2 handle

#define NUM_SAMPLES 1000
uint8_t here = 0;
uint8_t rxd = 0;


uint16_t Read_ADC1(void) {
    uint8_t rxData[2];          // Buffer to receive ADC data
    uint16_t adcValue;

    // Pull CS low to initiate communication with the ADC
    HAL_GPIO_WritePin(ADC1_CS_PORT, ADC1_CS_PIN, GPIO_PIN_RESET);

    // Receive 16 bits (2 bytes) of data from the ADC
    HAL_StatusTypeDef hal_status;
    hal_status = HAL_SPI_Receive(&hspi1, rxData, 2, 50);
    if (hal_status != HAL_OK) {
    	return;
    }
//    if (HAL_SPI_Receive(&hspi1, rxData, 2, 10) != HAL_OK) {  // Use a shorter timeout (10 ms)
//        HAL_GPIO_WritePin(ADC1_CS_PORT, ADC1_CS_PIN, GPIO_PIN_SET);
//        here++;
//        return 0xFFFF;  // Return error code if SPI communication fails
//    }

    // Pull CS high to end communication
    HAL_GPIO_WritePin(ADC1_CS_PORT, ADC1_CS_PIN, GPIO_PIN_SET);
    HAL_Delay(1);  // Allow time for the ADC to reset if needed

    // Combine received bytes into a 16-bit result
    rxd = rxData[0];

    adcValue = ((rxData[0] << 8) | rxData[1]);

    // If you find that shifting is required, apply the right shift to isolate the 12-bit result:
    // adcValue >>= 4;

    return adcValue;
}

uint16_t Read_ADC1_Average(void) {
    uint32_t sum = 0;
    uint16_t average;

    // Take NUM_SAMPLES readings and accumulate the results
    for (int i = 0; i < NUM_SAMPLES; i++) {
        uint16_t sample = Read_ADC1();

        // Skip invalid readings to avoid averaging incorrect values
//        if (sample != 0xFFFF) {
//        	int j = i;
//            sum += sample;
//        }

        sum += sample;

        HAL_Delay(1); // Small delay between samples if necessary
    }

    // Calculate the average ADC value, with a check to avoid division by zero
    average = sum / NUM_SAMPLES;

    return average;
}





