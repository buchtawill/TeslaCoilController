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
#define MAX_PULSE_WIDTH 125 //us
#define MIN_PULSE_WIDTH 5

#define MAX_FREQUENCY 800 //Hz
#define MAX_TIME_ON 500 //ms
#define MAX_TIME_OFF 500 //ms



void setTimerFrequencyPulseWidth(TIM_HandleTypeDef* pTim, uint16_t freq, uint16_t pulseWidth, uint32_t channel);
void startTimer(TIM_HandleTypeDef* pTim, uint32_t channel);
void stopTimer(TIM_HandleTypeDef* pTim, uint32_t channel);


#endif /* INC_TIMERS_H_ */
