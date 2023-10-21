/*
 * I2C_LCD.c
 *
 *  Created on: Jul 4, 2022
 *      Author: WBuchta
 *
 * 			7		6		5		4		3		2		1		0
 * 			db7		db6		db5		db4		BT		E		RW		RS
 * 			db3		db2		db1		db0		BT		E		RW		RS
 */

#include "I2C_LCD.h"



//HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
//HAL_I2C_Master_Receive

/*
 * Initializes LCD struct, resets the LCD, and sets it to 4 bit mode
 */
HAL_StatusTypeDef initLCD(LCD *dev, I2C_HandleTypeDef *handle, uint8_t nRows, uint8_t nCols, uint8_t address){
	dev->cols = nCols;
	dev->rows = nRows;
	dev->addr = address<<1;
	dev->handle = *handle;

	//page 46 of datasheet
	HAL_Delay(50);
	HAL_StatusTypeDef stat = write4BitsToInstructionReg(dev, 0b0000);
	HAL_Delay(50);
	write4BitsToInstructionReg(dev, 0b0010);
	HAL_Delay(50);
	//	write4BitsToInstructionReg(dev, 0b0010);
	//	HAL_Delay(50);

	//clear display
	stat |= writeToRegister(dev, 0b00000001, INSTRUCTION);
	HAL_Delay(50);

	stat |= writeToRegister(dev, 0b00000010, INSTRUCTION);
	HAL_Delay(50);

	stat |= writeToRegister(dev, 0b00001101, INSTRUCTION);
	HAL_Delay(50);

	return stat;
}

/**
 * @brief	sets the ddram address in a more familiar coordinate system.
 * 			top left corner is (0,0), bottom right is (19,3)
 * 			row 0: 0  1  2 ...19
 * 			row 1: 40 42 43...59
 * 			row 2: 20 22 23...39
 * 			row 3: 60 61 62...79
 */
HAL_StatusTypeDef setCursor(LCD *dev, uint8_t col, uint8_t row){
	if(row < 0 || row > (dev->rows - 1) || col < 0 || col > (dev->cols - 1))return HAL_ERROR;
	uint8_t addr = 0;
	if(row == 0) addr = col;
	else if (row == 1) addr = col+64;
	else if(row == 2) addr = col+20;
	else if(row == 3) addr = col+84;

	return setDDRAMAddress(dev, addr);
}

/**
 *	@brief	prints a given char array to the screen. Set the position first.
 */
HAL_StatusTypeDef LCDPrint(LCD *dev, char *pString){
	uint8_t len = strlen(pString);
	HAL_StatusTypeDef stat = HAL_OK;
	for(int i = 0; i<len; i++){
		stat |= writeToRegister(dev, *pString, DATA);
		pString++;
		HAL_Delay(1);
		//		delayMicroseconds(50); //takes 37 microseconds
	}
	return stat;
}

HAL_StatusTypeDef LCDPrintNumber(LCD *dev, uint16_t num, uint8_t col, uint8_t row, uint8_t spaceNum){

	int digits = 0;
	uint16_t n = num;
	char numberString[12];

	while (n != 0) {
		n /= 10;
		digits++;
	}

	if (num == 0) {
		digits = 1;
	}

	sprintf(numberString, "%0*d", spaceNum , num);

	HAL_StatusTypeDef stat = setCursor(dev, col, row);

	stat |= LCDPrintAtPos(dev, numberString, col, row);



	return stat;
}

HAL_StatusTypeDef LCDPrintAtPos(LCD *dev, char *pString, uint8_t col, uint8_t row){
	//	uint8_t len = strlen(pString);
	HAL_StatusTypeDef stat = setCursor(dev, col, row);
	return stat | LCDPrint(dev, pString);
}
/**
 * @brief	Writes a byte to a given DDRAM address
 * @param	byte - address of character in CGROM (see table in datasheet)
 * @param	addr - address of DDRAM to write to (which character on LCD)
 */
HAL_StatusTypeDef writeToDDRAMAddress(LCD *dev, uint8_t addr, uint8_t byte){
	HAL_StatusTypeDef stat = setDDRAMAddress(dev, addr);
	return stat | writeToRegister(dev, byte, DATA);
}

/**
 * @brief	sets the DDRAM address (cursor position). Address is 7 bits
 */
HAL_StatusTypeDef setDDRAMAddress(LCD *dev, uint8_t addr){
	//				  			command    7 bits of address
	uint8_t dataByte = SET_DDRAM_ADDR_BIT | (addr & 0b01111111);
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, INSTRUCTION);
	return bruh;
}

/**
 * @brief	Clears the display, resets any shift that may have been present, and
 * 			sets the cursor to home (top left corner)
 * 			7		6		5		4		3		2		1		0
 * 	byte1	db7		db6		db5		db4		BT		EN		RW		RS
 * 	byte2	db3		db2		db1		db0
 */
HAL_StatusTypeDef clearDisplay(LCD *dev){
	uint8_t dataByte = 0b00000001; //instruction from datasheet
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}

HAL_StatusTypeDef LCDCursorOnBlinkOff(LCD *dev){
	uint8_t dataByte = 0b00001110;
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}
HAL_StatusTypeDef LCDCursorOnBlinkOn(LCD *dev){
	uint8_t dataByte = 0b00001111;
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}
HAL_StatusTypeDef LCDCursorOffBlinkOn(LCD *dev){
	uint8_t dataByte = 0b00001101;
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}
HAL_StatusTypeDef LCDCursorOffBlinkOff(LCD *dev){
	uint8_t dataByte = 0b00001100;
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}
HAL_StatusTypeDef LCDDisplayOn(LCD *dev){
	uint8_t dataByte = 0b00001100;
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}
HAL_StatusTypeDef LCDDisplayOff(LCD *dev){
	uint8_t dataByte = 0b00001000;
	HAL_StatusTypeDef bruh = writeToRegister(dev, dataByte, 0);
	HAL_Delay(1);
	return bruh;
}

/**
 * @brief	mid level function writes db0-db7 (byte) to a register (rs)
 * @param 	dev - pointer to the LCD object
 * @param	byte - what to send to pins db0-db7
 * @param	rs - register select. 1 for data, 0 for instruction
 */
HAL_StatusTypeDef writeToRegister(LCD *dev, uint8_t byte, uint8_t rs){
	//	uint8_t first  = (byte & 0xF0)        | rs<<RS | 1<<BT;
	//	uint8_t second = ((byte & 0x0F) << 4) | rs<<RS | 1<<BT;
	//	first  &= ~(1<<RW);
	//	second &= ~(1<<RW);	//make sure write is high
	//	HAL_StatusTypeDef bruh = writeAByte(dev, first);
	//	bruh |= writeAByte(dev, second);
	HAL_StatusTypeDef bruh = HAL_OK;
	if(rs == INSTRUCTION){
		bruh = write4BitsToInstructionReg(dev, (byte & 0xF0) >> 4);
		bruh |= write4BitsToInstructionReg(dev, byte & 0x0F);
	}
	else if(rs == DATA){
		bruh = write4BitsToDataReg(dev, (byte & 0xF0) >> 4);
		bruh |= write4BitsToDataReg(dev, byte & 0x0F);
	}
	return bruh;
}

/**
 * @brief	puts the LS nibble of bits onto db4-7
 */
HAL_StatusTypeDef write4BitsToInstructionReg(LCD *dev, uint8_t bits){
	uint8_t expanderVal = 0;
	expanderVal |= (bits & 0x0F) << 4;
	expanderVal |= 1 << BT;
	HAL_StatusTypeDef stat = HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &expanderVal, 1, 1000);
	expanderVal |= 1<<EN;
	stat |= HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &expanderVal, 1, 1000);
	expanderVal &= ~(1<<EN);
	return stat | HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &expanderVal, 1, 1000);
}

HAL_StatusTypeDef write4BitsToDataReg(LCD *dev, uint8_t bits){
	uint8_t expanderVal = 0;
	expanderVal |= (bits & 0x0F)<<4;
	expanderVal |= 1 << BT | 1 << RS;
	HAL_StatusTypeDef stat = HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &expanderVal, 1, 1000);
	expanderVal |= 1<<EN;
	stat |= HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &expanderVal, 1, 1000);
	expanderVal &= ~(1<<EN);
	return stat | HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &expanderVal, 1, 1000);
}
/**
 * @brief	low level function to write a byte to the expander and pulse the enable pin
 */
HAL_StatusTypeDef writeAByte(LCD *dev, uint8_t byte){
	HAL_StatusTypeDef stat;
	byte &= ~(1<<EN);	//Make sure EN bit is off
	//byte |= 1 << BT;
	stat = HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &byte, 1, 1000);
	byte |= 1<<EN;
	delayMicroseconds(10);
	stat |= HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &byte, 1, 1000);
	byte &= ~(1<<EN);
	delayMicroseconds(10);
	return stat | HAL_I2C_Master_Transmit(&(dev->handle), dev->addr, &byte, 1, 1000);
}

