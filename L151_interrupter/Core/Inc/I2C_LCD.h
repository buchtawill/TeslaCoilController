/*
 * I2C_LCD.h
 *
 *  Created on: Jul 4, 2022
 *      Author: WBuchta
 *  This would be a lot nicer in c++.....Figured that out after writing everything
 *  *** FOR ROM CODE A00 ***
 *
 *	*** SCREEN SIZES OTHER THAN 20 BY 4 NOT IMPLEMENTED ***
 */

#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_

#include "main.h"
#include "stm32l1xx_hal.h"
#include "string.h"
#include <stdio.h>


/*
 * Send high bits then low bits
 * 			  7		6		5		4		3		2		1		0
 * 			db7		db6		db5		db4		BT		EN		RW		RS
 * 			db3		db2		db1		db0
 */
#define RS	0
#define RW	1
#define EN	2
#define BT	3

#define MAX_ADDR_1602
#define MAX_ADDR_2004 80

#define DATA			1
#define INSTRUCTION 	0

//Other characters
#define O_FACE 		0b11101111
#define LCD_PI		0b11110111
#define LCD_OMEGA	0b11110100
#define LCD_THETA	0b11110010
#define LCD_SIGMA	0b11110110
#define LCD_PI		0b11110111
#define LCD_XBAR	0b11111000

/*
 * Instruction Set
 * R/W will also always be low as I won't be reading any data ... the functions will delay
 * 		DB7-DB0 is listed. In 4 bit mode, send DB7-4, then 3-0
 * 	write character code into DDRAM
 * 	Set DDRAM address by sending {1,A6-A0}. Put RS to 1, then write ascii code
 */
#define LCD_CLEAR_BIT 			(1 << 0) //clears DDRAM, sets address counter to 0
#define RET_HOME_BIT			(1 << 1) //sets address counter to 0 and removes shift
#define DISP_ON_OFF_BIT			(1 << 3) //db2: display on/off. db1: cursor on/off. db0: blink on/off
#define CURSOR_DISP_SHIFT_BIT	(1 << 4) //db3: screen/cursor, db2: Right / left
#define SET_DDRAM_ADDR_BIT		(1 << 7) //db6-0: address value



typedef struct {
	uint8_t rows;
	uint8_t cols;
	uint8_t addr;
	I2C_HandleTypeDef handle;
}LCD;

//exported function prototypes: high level functions for user
HAL_StatusTypeDef initLCD(LCD *dev, I2C_HandleTypeDef *handle, uint8_t nRows, uint8_t nCols, uint8_t address);

HAL_StatusTypeDef clearDisplay(LCD *dev);
HAL_StatusTypeDef setCursor(LCD *dev, uint8_t col, uint8_t row);
HAL_StatusTypeDef LCDPrint(LCD *dev, char *pString);
HAL_StatusTypeDef LCDPrintAtPos(LCD *dev, char *pString, uint8_t col, uint8_t row);
HAL_StatusTypeDef LCDPrintNumber(LCD *dev, uint16_t num, uint8_t col, uint8_t row, uint8_t spaceNum);

HAL_StatusTypeDef LCDDisplayOn(LCD *dev);
HAL_StatusTypeDef LCDDisplayOff(LCD *dev);

HAL_StatusTypeDef LCDCursorOnBlinkOff(LCD *dev);
HAL_StatusTypeDef LCDCursorOnBlinkOn(LCD *dev);
HAL_StatusTypeDef LCDCursorOffBlinkOn(LCD *dev);
HAL_StatusTypeDef LCDCursorOffBlinkOff(LCD *dev);

//low level, bit pushing functions
HAL_StatusTypeDef writeToDDRAMAddress(LCD *dev, uint8_t addr, uint8_t byte);
HAL_StatusTypeDef setDDRAMAddress(LCD *dev, uint8_t addr);
HAL_StatusTypeDef pulseEnablePin(LCD *dev, uint8_t currentData);
HAL_StatusTypeDef writeToRegister(LCD *dev, uint8_t command, uint8_t rs);
HAL_StatusTypeDef writeAByte(LCD *dev, uint8_t byte);
HAL_StatusTypeDef write4BitsToDataReg(LCD *dev, uint8_t bits);
HAL_StatusTypeDef write4BitsToInstructionReg(LCD *dev, uint8_t bits);



#endif /* INC_I2C_LCD_H_ */
