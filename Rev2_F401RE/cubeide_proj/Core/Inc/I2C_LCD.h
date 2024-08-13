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
#include "stm32f4xx_hal.h"
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

#define LCD_DATA_REG	1
#define LCD_INSTR_REG 	0

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
	uint8_t rows; //Number of rows
	uint8_t cols; //Number of columns
	uint8_t addr; //I2C address
	I2C_HandleTypeDef handle;
}LCD;

/**
 * @brief Initialize the LCD struct, reset the LCD, and set it to 4 bit mode
 * @param dev pointer to the LCD struct
 * @param handle handle to the I2C 
 * @param nRows number of rows the LCD has
 * @param nCols number of columns the LCD has
 * @param address the I2C address of the IO expander connected to the LCD
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef initLCD(LCD *dev, I2C_HandleTypeDef *handle, uint8_t nRows, uint8_t nCols, uint8_t address);

/**
 * @brief	Clears the display, resets any shift that may have been present, and
 * 			sets the cursor to home (top left corner)
 * 			7		6		5		4		3		2		1		0
 * 	byte1	db7		db6		db5		db4		BT		EN		RW		RS
 * 	byte2	db3		db2		db1		db0
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef clearDisplay(LCD *dev);

/**
 * @brief Set the LCD's cursor to (col, row)
 * @param dev pointer to the LCD struct
 * @param col column to set the cursor to
 * @param row row to set the cursor to
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef setCursor(LCD *dev, uint8_t col, uint8_t row);

/**
 * @brief Write a string to the LCD at the current cursor position. Set the position first with setCursor().
 * @param dev pointer to the LCD struct
 * @param pString null terminated string to write to the LCD
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDPrint(LCD *dev, char *pString);

/**
 * @brief Write a string to the LCD at the specified position
 * @param dev pointer to the LCD struct
 * @param pString null terminated string to write to the LCD
 * @param col column to write the string at
 * @param row row to write the string at
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDPrintAtPos(LCD *dev, char *pString, uint8_t col, uint8_t row);

/**
 * @brief Write a number to the LCD at the specified position
 * @param dev pointer to the LCD struct
 * @param num number to write to the LCD
 * @param col column to write the number at
 * @param row row to write the number at
 * @param spaceNum number of spaces to pad the number with
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDPrintNumber(LCD *dev, uint16_t num, uint8_t col, uint8_t row, uint8_t spaceNum);

/**
 * @brief Turn the display on
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDDisplayOn(LCD *dev);

/**
 * @brief Turn the display off
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDDisplayOff(LCD *dev);

/**
 * @brief Turn the cursor on and blink off
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDCursorOnBlinkOff(LCD *dev);

/**
 * @brief Turn the cursor on and blink on
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDCursorOnBlinkOn(LCD *dev);

/**
 * @brief Turn the cursor off and blink on
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDCursorOffBlinkOn(LCD *dev);

/**
 * @brief Turn the cursor off and blink off
 * @param dev pointer to the LCD struct
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCDCursorOffBlinkOff(LCD *dev);

//
//low level, bit pushing functions
//

/**
 * @brief Write byte to addr 
 * @param dev pointer to the LCD struct
 * @param addr address to write to
 * @param byte byte to write
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef writeToDDRAMAddress(LCD *dev, uint8_t addr, uint8_t byte);

/**
 * @brief Set the DDRAM address (cursor position). Address is 7 bits
 * @param dev pointer to the LCD struct
 * @param addr address to set the DDRAM to
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef setDDRAMAddress(LCD *dev, uint8_t addr);

/**
 * @brief Write a byte to the LCD
 * @param dev pointer to the LCD struct
 * @param command byte to write to the LCD
 * @param rs register select. 1 for data, 0 for instruction
 */
HAL_StatusTypeDef writeToRegister(LCD *dev, uint8_t command, uint8_t rs);

/**
 * @brief low level function to write a byte to the expander and pulse the enable pin
 * @param dev pointer to the LCD struct
 * @param byte byte to write to the LCD
 */
HAL_StatusTypeDef writeAByte(LCD *dev, uint8_t byte);

/**
 * @brief write the lower 4 bits of bits to the data register
 * @param dev pointer to the LCD struct
 * @param bits lower 4 bits to write to the data register
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef write4BitsToDataReg(LCD *dev, uint8_t bits);

/**
 * @brief write the lower 4 bits of bits to the instruction register
 * @param dev pointer to the LCD struct
 * @param bits lower 4 bits to write to the instruction register
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef write4BitsToInstructionReg(LCD *dev, uint8_t bits);



#endif /* INC_I2C_LCD_H_ */
