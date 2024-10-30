/*
 * seven_seg.h
 *
 *  Created on: Oct 25, 2024
 *      Author: Will Buchta
 */

/* 	There are 6 seven-segment displays, each with 4 digits.
	They are displayed in the following order:
	Current0	Current1
	Voltage0	Voltage1
	Temp0		Temp1

	Each 7-seg display has a shift register associated with it. A 7th shift register
	controls which digits to activate.

	When sending messages, bytes must be sent in the following order
	  digits, temp[1], voltage[1], current[1], current[0], voltage[0], temp[0]


*/

#ifndef INC_SEVEN_SEG_H_
#define INC_SEVEN_SEG_H_

#include <stdint.h>

#include "main.h"

// DIGITS(0) is the leftmost digit
#define DECIMAL_POINT 	BIT7
#define DIGITS(i) 		((BIT4 | BIT0) << i)

/**
 * Outgoing SPI message struct containing data for 7 shift registers.
 * current[0] is the display on the left, current[1] is the display on the right.
 * Same is true for voltage and temp.
 * digits controls which digits are to be on.
 *
 * Set digits to be one of DIGITS(i), where 0 is the left digit, 3 is the rightmost digit
 */
typedef struct outgoing_msg{
	uint8_t current[2];
	uint8_t voltage[2];
	uint8_t temp[2];
	uint8_t digits;
}OutgoingMessage;


/**
 * @param value 		16 bit value to display
 * @param val_float 	32 bit float to display
 * @param decimal_pos 	position of the decimal point, if float
 * @param display_float whether or not this seg7 is meant for floats
 * @param digits 		array of digits
 */
typedef struct seg7{
	uint16_t value;
	float    val_float;
	uint8_t  decimal_pos;
	uint8_t display_float;

	uint8_t  digits[4];
}seg7;

typedef struct coil{
	seg7 temp;
	seg7 voltage;
	seg7 current;
}CoilGroup;

/**
 * Calculate the correct segments to light to display a number
 * on a seven-segment display.
 */
uint8_t num_to_seg(uint8_t num_to_display);

/**
 * Populate p_msg with info from p_coils and the current digit
 * @param tx_buf pointer to an array of 8 bit ints. Must be 7 bytes
 * @param p_coils pointer to an array of Coil structs (ASSUMING 2 COILS!)
 * @param current_digit the current digit that is activated (0-4)
 */
void populate_tx_buf(uint8_t *tx_buf, CoilGroup *p_coils, uint8_t current_digit);

/**
 * Update the value and digits array of p_seg to match val
 */
void set_seg7_int(seg7 *p_seg, uint16_t val);

// Disable all segments
void turn_off_display(seg7 *p_seg);

/**
 * Update the value and digits array of p_seg to match val, but as a float
 */
void set_seg7_float(seg7 *p_seg, float val);

#endif /* INC_SEVEN_SEG_H_ */



