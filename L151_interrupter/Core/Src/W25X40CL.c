/*
 * W25X40CL.c
 *
 *  Created on: Jul 4, 2022
 *      Author: WBuchta
 *
 *  Driver Library for the W25X40CL flash memory chip: https://www.winbond.com/resource-files/w25x40cl_f%2020140325.pdf
 *  Memory is organized as 8 blocks of 16 sectors (each sector being 4KB)
 *	0x 00    00    00    00
 *
 */
#include "W25X40CL.h"
#include "main.h"
/*
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,
                                          uint32_t Timeout);
 */

/*
 * @brief	Initializes the W25X40CL struct
 * @param	dev the struct
 * @param	_spiHandle pointer to the SPI struct that the chip is connected to
 * @param	ssPin slave select GPIO pin
 * @param 	ssBank the bank of the ss gpio
 */
void flashInit(W25X40CL *dev, SPI_HandleTypeDef *_spiHandle, uint32_t ssPin, GPIO_TypeDef  *ssBank){
	dev->spiHandle = _spiHandle;
	dev->SS_Bank = ssBank;
	dev->SS_Pin = ssPin;
}

/*
 * @brief	enables memory locations to be programmed
 */
HAL_StatusTypeDef flashWriteEnable(W25X40CL *dev){
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_StatusTypeDef stat;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat = HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)WRITE_EN, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_SET);

	return stat;
}


HAL_StatusTypeDef flashWriteEnableVolatile(W25X40CL *dev){
	HAL_StatusTypeDef stat;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat = HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)WRITE_EN_VOLATILE, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_SET);

	return stat;
}

/**
 * @brief	disables the write function on the chip
 */
HAL_StatusTypeDef flashWriteDisable(W25X40CL *dev){
	HAL_StatusTypeDef stat;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat = HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)WRITE_DISABLE, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_SET);

	return stat;
}

/**
 * @brief	Reads the value of the status register
 *			Bit#	function
 *			0		BUSY
 *			1		Write enable latch
 *			2-4		Block protect bits
 *			5		Top/bottom protect
 *			6		Reserved
 *			7		Status register protect
 */
HAL_StatusTypeDef flashReadStatusRegister(W25X40CL *dev, uint8_t* byte){
	HAL_StatusTypeDef stat;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat  = HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)READ_STATUS_REG, 1, 1000);
	stat |= HAL_SPI_Receive(dev->spiHandle, byte, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);

	return stat;
}

/**
 * @brief	returns the BUSY bit in the status register. 1 when busy
 */
int isBusy(W25X40CL *dev){
	uint8_t buf;
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)READ_STATUS_REG, 1, 1000);
	HAL_SPI_Receive(dev->spiHandle, &buf, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_SET);
	return (buf&0x01);
}

/**
 * @brief	Writes to the status register. Enables writing first
 *			Bit#	function
 *			0		BUSY					read only
 *			1		Write enable latch		read only
 *			2-4		Block protect bits
 *			5		Top/bottom protect
 *			6		Reserved
 *			7		Status register protect
 *
 */
HAL_StatusTypeDef flashWriteStatusRegister(W25X40CL *dev, uint8_t byte){
	HAL_StatusTypeDef stat = flashWriteEnable(dev);

	uint8_t txData[2] = {WRITE_STATUS_REG, byte};
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat = HAL_SPI_Transmit(dev->spiHandle, txData, 2, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_SET);

	return stat;
}

/**
 * @brief	read nBytes number of bytes from a start address (addr) into an array (data)
 * @param	dev pointer to the flash struct
 * @param	addr the address that data starts at
 * @param	pRxData pointer to a byte or array
 * @param	nBytes number of bytes to read
 */
HAL_StatusTypeDef flashReadData(W25X40CL *dev, uint32_t addr, uint8_t* pRxData, uint32_t nBytes){
	HAL_StatusTypeDef stat;

	uint8_t txData[4];
	txData[0] = READ_DATA;
	txData[1] = addr & 0xFF0000;
	txData[2] = addr & 0x00FF00;
	txData[3] = addr & 0x0000FF;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat  = HAL_SPI_Transmit(dev->spiHandle, txData, 4, 5000);
	stat |= HAL_SPI_Receive(dev->spiHandle, pRxData, nBytes, 5000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_SET);

	return stat;
}

//not gonna implement these bad bois yet
//HAL_StatusTypeDef flashFastReadData(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes){
//	HAL_StatusTypeDef stat;
//}
//
//
//HAL_StatusTypeDef flashFastReadDualOutput(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes){
//	HAL_StatusTypeDef stat;
//}
//
//
//HAL_StatusTypeDef flashFastReadDualIO(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes){
//	HAL_StatusTypeDef stat;
//}

/**
 * @brief	program up to 256 bytes of the flash memory. addr is the first memory
 * 				address, the chip will increment the address for each byte. Takes 400-800us.
 * 				flashWriteEnable must be called prior to this
 * @param	dev pointer to the flash struct containing spi info
 * @param	addr first address to program
 * @param	pData pointer to the data to be flashed (array or byte)
 * @param	nBytes number of bytes to send (size of pData)
 */
HAL_StatusTypeDef flashPageProgram(W25X40CL *dev, uint32_t addr, uint8_t* pData, uint8_t nBytes){
	HAL_StatusTypeDef stat;

	uint8_t txData[4];
	txData[0] = PAGE_PROGRAM;
	txData[1] = addr & 0xFF0000;
	txData[2] = addr & 0x00FF00;
	txData[3] = addr & 0x0000FF;
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);

	stat  = HAL_SPI_Transmit(dev->spiHandle, txData, 4, 1000);
	stat |= HAL_SPI_Receive(dev->spiHandle, pData, nBytes, 5000);

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;

}

/**
 * @brief	Sets all memory in a specified 4KB sector to all 1's. Sends a write enable command
 * 			before issuing sector erase command. TAKES 30-300 ms
 * @param   dev struct containing spi info
 * @param	addr address of the sector to erase (see figure 2 in datasheet)
 */
HAL_StatusTypeDef flashSectorErase(W25X40CL *dev, uint32_t addr){
	HAL_StatusTypeDef stat = flashWriteEnable(dev);
	uint8_t txData[4];
	txData[0] = SECTOR_ERASE;
	txData[1] = addr & 0xFF0000;
	txData[2] = addr & 0x00FF00;
	txData[3] = addr & 0x0000FF;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat |= HAL_SPI_Transmit(dev->spiHandle, txData, 4, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	Sets all memory in half a block (32KB) to all 1's. 1 block is 16 sectors.
 * 				chip contains 8 blocks. TAKES 120-800ms
 * @param 	addr the address of the block (see figure 2 in datasheet)
 */
HAL_StatusTypeDef flashBlockErase32KB(W25X40CL *dev, uint32_t addr){
	HAL_StatusTypeDef stat = flashWriteEnable(dev);
	uint8_t txData[4];
	txData[0] = BLOCK_ERASE_32;
	txData[1] = addr & 0xFF0000;
	txData[2] = addr & 0x00FF00;
	txData[3] = addr & 0x0000FF;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat = HAL_SPI_Transmit(dev->spiHandle, txData, 4, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	erases a 64KB block to all 1's. Chip contains 8 blocks. TAKES 150-1000ms
 * @param	addr address of the block (see figure 2 in datasheet)
 */
HAL_StatusTypeDef flashBlockErase64KB(W25X40CL *dev, uint32_t addr){
	HAL_StatusTypeDef stat = flashWriteEnable(dev);
	uint8_t txData[4];
	txData[0] = BLOCK_ERASE_64;
	txData[1] = addr & 0xFF0000;
	txData[2] = addr & 0x00FF00;
	txData[3] = addr & 0x0000FF;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat |= HAL_SPI_Transmit(dev->spiHandle, txData, 4, 5000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	erases the entire chip to all 1's. Takes 1-4 seconds
 */
HAL_StatusTypeDef flashChipErase(W25X40CL *dev){
	HAL_StatusTypeDef stat = flashWriteEnable(dev);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat |= HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)CHIP_ERASE, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	puts the chip into a low power mode, reducing current draw
 * 			from 10 to 1 uA(typ)
 */
HAL_StatusTypeDef flashPowerDown(W25X40CL *dev){
	HAL_StatusTypeDef stat;
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat = HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)POWER_DOWN, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	puts the chip into normal mode, reading the device ID in the process
 * @param	pID pointer to where to put the ID
 */
HAL_StatusTypeDef flashReleasePowerDown(W25X40CL *dev, uint8_t *pID){
	HAL_StatusTypeDef stat;

	uint8_t txData[4];
	txData[0] = RELEASE_POWER_DOWN;
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat  = HAL_SPI_Transmit(dev->spiHandle, txData, 3, 1000);
	stat |= HAL_SPI_Receive(dev->spiHandle, pID, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	reads the manufacturer ID (0xEF) and device ID (0x12)
 */
HAL_StatusTypeDef flashReadID(W25X40CL *dev, uint8_t *pMID, uint8_t *pDID){
	HAL_StatusTypeDef stat;
	uint8_t txData[4];
	txData[0] = MAN_ID_DEV_ID;
	txData[1] = 0;
	txData[2] = 0;
	txData[3] = 0;
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat  = HAL_SPI_Transmit(dev->spiHandle, txData, 4, 1000);
	stat |= HAL_SPI_Receive(dev->spiHandle, pMID, 1, 1000);
	stat |= HAL_SPI_Receive(dev->spiHandle, pDID, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	returns a 64 bit unique serial ID. Can be used to stop fraud
 */
HAL_StatusTypeDef flashReadUniqueID(W25X40CL *dev, uint8_t *pID){
	HAL_StatusTypeDef stat;
	uint8_t txData[5]; //4 dummy bytes
	txData[0] = READ_UNIQUE_ID;

	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat  = HAL_SPI_Transmit(dev->spiHandle, txData, 5, 1000);
	stat |= HAL_SPI_Receive(dev->spiHandle, pID, 8, 5000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}

/**
 * @brief	returns the manufacturer ID (0xEF), memory type (0x30), and capacity(0x13).
 * 			memory and capacity usually as one 16 bit integer, but here I kept them separate.
 */
HAL_StatusTypeDef flashReadJEDECID(W25X40CL *dev, uint8_t *pMID, uint8_t *pMemType, uint8_t *pCapacity){
	HAL_StatusTypeDef stat;
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	stat  = HAL_SPI_Transmit(dev->spiHandle, (uint8_t *)READ_JEDEC_ID, 1, 1000);
	stat |= HAL_SPI_Receive(dev-> spiHandle, pMID, 1, 1000);
	stat |= HAL_SPI_Receive(dev-> spiHandle, pMemType, 1, 1000);
	stat |= HAL_SPI_Receive(dev-> spiHandle, pCapacity, 1, 1000);
	HAL_GPIO_WritePin(dev->SS_Bank, dev->SS_Pin, GPIO_PIN_RESET);
	return stat;
}



