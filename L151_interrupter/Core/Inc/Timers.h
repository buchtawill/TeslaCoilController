/*
 * Timers.h
 *
 *  Created on: Jul 16, 2022
 *      Author: WBuchta
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#include "main.h"

#define CPU_CLK 32000000
#define MAX_PULSE_WIDTH 150 //us
#define MIN_PULSE_WIDTH 5

#define MAX_FREQUENCY      		1180 // Hz: D6 = 1174
#define ONTIME_NERFING_BOUND	700  // Upper bound of no restriction for ontime. Above this, divide on time by 2
#define DUTY_CYCLE_LIMIT		10   // 10% limit on duty cycle
#define MAX_TIME_ON      		500  // ms, for burst mode
#define MAX_TIME_OFF  			500  // ms, for burst mode



void setTimerFrequencyPulseWidth(TIM_HandleTypeDef* pTim, uint16_t freq, uint16_t pulseWidth, uint32_t channel);
void startTimer(TIM_HandleTypeDef* pTim, uint32_t channel);
void stopTimer(TIM_HandleTypeDef* pTim, uint32_t channel);


#endif /* INC_TIMERS_H_ */
