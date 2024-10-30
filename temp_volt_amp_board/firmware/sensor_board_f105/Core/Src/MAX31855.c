#include "max31855.h" // ill make this ...```

extern SPI_HandleTypeDef hspi3; // SPI3 handle

void MAX31855_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MAX31855_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MAX31855_CS_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(MAX31855_CS_PORT, MAX31855_CS_PIN, GPIO_PIN_SET);

}
// based off of (https://theembeddedthings.com/stmp32/stm32-and-max31855-type-k-thermocouple-interface/) 's implementation
uint32_t MAX31855_ReadRaw(void) {
    uint8_t rxData[4] = {0};  // Buffer to store received data
    uint32_t rawData = 0;

    // Pull CS low to start communication
    HAL_GPIO_WritePin(MAX31855_CS_PORT, MAX31855_CS_PIN, GPIO_PIN_RESET);
    //used to debug error for MAX31855_GetTemperature
    if (HAL_SPI_Receive(&hspi3, rxData, 4, HAL_MAX_DELAY) != HAL_OK) {
        // If communication fails, deselect CS and return error code
        HAL_GPIO_WritePin(MAX31855_CS_PORT, MAX31855_CS_PIN, GPIO_PIN_SET);
        return 0xFFFFFFFF;
    }

    // Pull CS high to end communication
    HAL_GPIO_WritePin(MAX31855_CS_PORT, MAX31855_CS_PIN, GPIO_PIN_SET);

    // Combine received bytes into a single 32-bit integer
    rawData = (rxData[0] << 24) | (rxData[1] << 16) | (rxData[2] << 8) | rxData[3];
    return rawData;
}

// Function to get temperature in Celsius from the MAX31855
float MAX31855_GetTemperature(void) {
    uint32_t rawData;
    int32_t totalTempValue = 0;
    int validSamples = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        rawData = MAX31855_ReadRaw();

        // Check for communication error or fault
        if (rawData == 0xFFFFFFFF || (rawData & 0x7)) {
            continue;
        }

        // Extract temperature in °C from the 14-bit signed temperature value
        int16_t tempValue = (rawData >> 18) & 0x3FFF;
        if (tempValue & 0x2000) {  // Check if temperature is negative
            tempValue |= 0xC000;   // Sign extend if negative
        }

        totalTempValue += tempValue;
        validSamples++;

        HAL_Delay(10); // Add a small delay between samples
    }

    if (validSamples == 0) {
        return -1.0f;  // Return error if no valid samples were read
    }

    // Calculate the average temperature value
    float averageTempValue = (float)totalTempValue / validSamples;

    // Convert the average temperature to Celsius (0.25°C resolution)
    return averageTempValue * 0.25f;
}


