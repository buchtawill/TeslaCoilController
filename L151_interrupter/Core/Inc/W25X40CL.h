/*
 * W25X40CL.h
 *
 *  Created on: Jul 4, 2022
 *      Author: WBuchta
 */

//		https://www.winbond.com/resource-files/w25x40cl_f%2020140325.pdf
//		512 KB Flash memory

#ifndef INC_W25X40CL_H_
#define INC_W25X40CL_H_

#include "stm32l1xx_hal.h" //needed for SPI


//Instruction set (page 15)

#define WRITE_EN            	0x06
#define WRITE_EN_VOLATILE   	0x50
#define WRITE_DISABLE       	0x04
#define READ_STATUS_REG     	0x05
#define WRITE_STATUS_REG    	0x01

#define READ_DATA				0x03
#define FAST_READ				0x0B
#define FAST_READ_DUAL_OUT		0x3B
#define FAST_READ_DUAL_IO		0xBB

#define PAGE_PROGRAM			0x02
#define SECTOR_ERASE			0x20
#define BLOCK_ERASE_32			0x52
#define BLOCK_ERASE_64			0xD8
#define CHIP_ERASE				0xC7
#define POWER_DOWN				0xB9
#define RELEASE_POWER_DOWN		0xAB	//power up

#define DEVICE_ID				0xAB
#define MAN_ID_DEV_ID			0x90
#define MAN_ID_DEV_ID_DUALIO	0x92
#define READ_JEDEC_ID			0x9F
#define READ_UNIQUE_ID			0x4B

//end instruction set

//Struct
typedef struct{
	SPI_HandleTypeDef *spiHandle; //SPI Struct
	GPIO_TypeDef  *SS_Bank; //GPIO bank (GPIOA, GPIOB...)
	uint32_t SS_Pin;  //GPIO pin
} W25X40CL;

//function prototypes
int isBusy(W25X40CL *dev);
void flashInit(W25X40CL *dev, SPI_HandleTypeDef *_spiHandle, uint32_t ssPin, GPIO_TypeDef *ssBank);
HAL_StatusTypeDef flashWriteEnable(W25X40CL *dev);
HAL_StatusTypeDef flashWriteEnableVolatile(W25X40CL *dev);
HAL_StatusTypeDef flashWriteDisable(W25X40CL *dev);
HAL_StatusTypeDef flashReadStatusRegister(W25X40CL *dev, uint8_t* byte);
HAL_StatusTypeDef flashWriteStatusRegister(W25X40CL *dev, uint8_t byte);
HAL_StatusTypeDef flashReadData(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes);
HAL_StatusTypeDef flashFastReadData(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes);
HAL_StatusTypeDef flashFastReadDualOutput(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes);
HAL_StatusTypeDef flashFastReadDualIO(W25X40CL *dev, uint32_t addr, uint8_t* data, uint32_t nBytes);
HAL_StatusTypeDef flashPageProgram(W25X40CL *dev, uint32_t addr, uint8_t* data, uint8_t nBytes);
HAL_StatusTypeDef flashSectorErase(W25X40CL *dev, uint32_t addr);
HAL_StatusTypeDef flashBlockErase32KB(W25X40CL *dev, uint32_t addr);
HAL_StatusTypeDef flashBlockErase64KB(W25X40CL *dev, uint32_t addr);
HAL_StatusTypeDef flashChipErase(W25X40CL *dev);
HAL_StatusTypeDef flashPowerDown(W25X40CL *dev);
HAL_StatusTypeDef flashReleasePowerDown(W25X40CL *dev, uint8_t *pID);
HAL_StatusTypeDef flashReadID(W25X40CL *dev, uint8_t *pMID, uint8_t *pDID);
HAL_StatusTypeDef flashReadUniqueID(W25X40CL *dev, uint8_t *pID);
HAL_StatusTypeDef flashReadJEDECID(W25X40CL *dev, uint8_t *pMID, uint8_t *pMemType, uint8_t *pCapacity);



//FlashStatus flashReadDeviceID(uint8_t* pID); //all functions return either success or failure enum



#endif /* INC_W25X40CL_H_ */
