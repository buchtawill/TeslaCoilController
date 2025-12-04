/*
 * MAX31855.h
 *
 *  Created on: Dec 3, 2025
 *      Author: rmranjitkar
 */

#ifndef SRC_MAX31855_H_
#define SRC_MAX31855_H_

#define NUM_SAMPLES 1			// Should be a 2^N constant

void MAX31855_Init(void);
uint32_t read_Raw(void);
float read_Temp(void);

static inline void CS_1_Select(void);
static inline void CS_1_Deselect(void);
static inline void CS_2_Select(void);
static inline void CS_2_Deselect(void);

#endif /* SRC_MAX31855_H_ */
