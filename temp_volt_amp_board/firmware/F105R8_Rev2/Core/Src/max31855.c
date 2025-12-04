/*
 * MAX31855.c
 *
 *  Created on: Dec 3, 2025
 *      Author: rmranjitkar
 */

#include "main.h"
#include "max31855.h"

extern SPI_HandleTypeDef hspi3;

void MAX31855_Init(void){

	// Pins are intialized in the MX_GPIO_INIT

	// Drive Chip select Signal High for IDLE
	HAL_GPIO_WritePin(SPI3_NSS_1_GPIO_Port, SPI3_NSS_1_Pin, GPIO_PIN_SET);		// Drive NSS 1 High
	HAL_GPIO_WritePin(SPI3_NSS_2_GPIO_Port, SPI3_NSS_2_Pin, GPIO_PIN_SET);		// Drive NSS 2 High
}

// Add a chip select value to take in a specific snesor
// Testing it on the first sensor for now
uint32_t read_Raw(void)
{
    uint8_t rx_Data[4] = {0};
    uint8_t tx_dummy[4] = {0, 0, 0, 0};
    uint32_t raw_Data = 0;

    // Select MAX31855
    HAL_GPIO_WritePin(SPI3_NSS_1_GPIO_Port, SPI3_NSS_1_Pin, GPIO_PIN_RESET);

    // Perform hlaf-duplex read (4 bytes)
    if (HAL_SPI_TransmitReceive(&hspi3,tx_dummy, rx_Data, 4, 100) != HAL_OK)
    {
        // Deselect chip
        HAL_GPIO_WritePin(SPI3_NSS_1_GPIO_Port, SPI3_NSS_1_Pin, GPIO_PIN_SET);
        return 0xFFFFFFFF;   // Error indicator
    }

    // Deselect MAX31855
    HAL_GPIO_WritePin(SPI3_NSS_1_GPIO_Port, SPI3_NSS_1_Pin, GPIO_PIN_SET);

    raw_Data =  ((uint32_t)rx_Data[0] << 24) |
                ((uint32_t)rx_Data[1] << 16) |
                ((uint32_t)rx_Data[2] << 8)  |
                ((uint32_t)rx_Data[3]);

    return raw_Data;
}


float read_Temp(void) {
    uint32_t raw_Data;
    int16_t raw_Temp;
    int32_t temp_Sum = 0;
    float averaged_Temp = 0;
    int valid_Samples = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {

    	// Read the raw data
        raw_Data = read_Raw();

        if (raw_Data == 0xFFFFFFFF || (raw_Data & 0x7))
        {
        	continue; // Skip Bad Samples
		}

        // Extract the temp in °C from the raw data bits
        raw_Temp = (raw_Data >> 18) & 0x3FFF;

        // sign extend if negative
        if (raw_Temp & 0x2000) {
            raw_Temp |= 0xC000;
        }

        // Sum the raw temps
        temp_Sum += raw_Temp;

        // Add Valid Samples
        valid_Samples++;

        HAL_Delay(10);
    }

    if (valid_Samples == 0)
    {
		return -1.0f;  // Return error if no valid samples were read
	}

    // Get the average temp
    averaged_Temp = (float)temp_Sum / NUM_SAMPLES;

    // Return the average temp in 0.25 °C resolution
    return averaged_Temp * 0.25f;   // convert to °C
}


// Chip Select Functions
static inline void CS_1_Select(void){
	HAL_GPIO_WritePin(SPI3_NSS_1_GPIO_Port, SPI3_NSS_1_Pin, GPIO_PIN_RESET);
}

static inline void CS_1_Deselect(void){
	HAL_GPIO_WritePin(SPI3_NSS_1_GPIO_Port, SPI3_NSS_1_Pin, GPIO_PIN_SET);
}

static inline void CS_2_Select(void){
	HAL_GPIO_WritePin(SPI3_NSS_2_GPIO_Port, SPI3_NSS_2_Pin, GPIO_PIN_RESET);
}

static inline void CS_2_Deselect(void){
	HAL_GPIO_WritePin(SPI3_NSS_2_GPIO_Port, SPI3_NSS_2_Pin, GPIO_PIN_SET);
}
