#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "stm32f4xx_hal.h"

#define CPU_CLK_HZ          84000000
#define MAX_PULSE_WIDTH     150 //microseconds
#define MIN_PULSE_WIDTH     10  //microseconds

/**
 * @brief increment the millisecond counter
 * @return None
 */
void increment_millis();

/**
 * @brief get the current millisecond counter
 * @return millisecond counter
 */
uint32_t get_millis();

/**
 * @brief Set the current millis timer to 0
 */
void reset_millis();

#endif //#ifndef __TIMERS_H__