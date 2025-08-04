/*
 * seven_seg.c
 *
 *  Created on: Oct 25, 2024
 *      Author: bucht
 */

#include "seven_seg.h"

// BIT7: decimal point
// BIT6: G
// BIT5: F
// BIT4: E
// BIT3: D
// BIT2: C
// BIT1: B
// BIT0: A

const uint8_t seg_map[16] = {
		0x3F, // 0
		0x06, // 1
		0x5B, // 2
		0x4F, // 3
		0x66, // 4
		0x6D, // 5
		0x7D, // 6
		0x07, // 7
		0x7F, // 8
		0x6F, // 9
		0x77, // A
		0x7C, // b
		0x39, // C
		0x5E, // d
		0x79, // E
		0x71  // F
};

uint8_t num_to_seg(uint8_t num_to_display){
	if(num_to_display > 15) return 0;
	else return seg_map[num_to_display];
}

void set_seg7_int(seg7 *p_seg, uint16_t val){
	p_seg->value = val;

	// Update the digits 
	// digits[0] is leftmost digit, thousands place
	p_seg->digits[0] = (val / 1000);
	p_seg->digits[1] = (val / 100) % 10;
	p_seg->digits[2] = (val / 10)  % 10;
	p_seg->digits[3] = (val % 10);

	// Remove leading zeros
	for (int i = 0; i < 4; i++){
		if(p_seg->digits[i] != 0) return;
		else p_seg->digits[i] = 255; // disable
	}
}

void turn_off_display(seg7 *p_seg){
	p_seg->digits[0] = 16;
	p_seg->digits[1] = 16;
	p_seg->digits[2] = 16;
	p_seg->digits[3] = 16;
}

void populate_tx_buf(uint8_t *tx_buf, CoilGroup *p_coils, uint8_t current_digit){
	if(current_digit > 3) return;
	//message order: digits, temp[1], voltage[1], current[1], current[0], voltage[0], temp[0]
	tx_buf[0] = DIGITS(current_digit);
	tx_buf[1] = num_to_seg(p_coils[1].temp.digits[current_digit]);
	tx_buf[2] = num_to_seg(p_coils[1].voltage.digits[current_digit]);
	tx_buf[3] = num_to_seg(p_coils[1].current.digits[current_digit]);
	tx_buf[4] = num_to_seg(p_coils[0].current.digits[current_digit]);
	tx_buf[5] = num_to_seg(p_coils[0].voltage.digits[current_digit]);
	tx_buf[6] = num_to_seg(p_coils[0].temp.digits[current_digit]);
}




