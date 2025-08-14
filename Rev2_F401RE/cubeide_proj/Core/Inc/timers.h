#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "stm32f4xx_hal.h"
#include "main.h"

#define CPU_CLK_HZ          64000000
#define MAX_PULSE_WIDTH     150 //microseconds
#define MIN_PULSE_WIDTH     10  //microseconds
#define MAX_FREQUENCY      		1180 // Hz: D6 = 1174
#define ONTIME_NERFING_BOUND	800  // Upper bound of no restriction for ontime. Above this, divide on time by 2
#define DUTY_CYCLE_LIMIT		10   // 10% limit on duty cycle
#define MAX_TIME_ON      		500  // ms, for burst mode
#define MAX_TIME_OFF  			500  // ms, for burst mode

void setTimerFrequencyPulseWidth(TIM_HandleTypeDef* pTim, uint16_t freq, uint16_t pulseWidth, uint32_t channel);
void startTimer(TIM_HandleTypeDef* pTim, uint32_t channel);
void stopTimer(TIM_HandleTypeDef* pTim, uint32_t channel);


#endif //#ifndef __TIMERS_H__
