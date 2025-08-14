#include "timers.h"
#include "main.h"


/**
 * @brief 	sets a given timer to a frequency of freq (Hz) and pulseWidth (us). Set pulsewidth to 0 to turn off
 * @param	pTim pointer to the timer struct
 * @param	freq frequency in hertz
 * @param	pulseWidth desired pulsewidth in microseconds. Be careful of low frequencies!
 */
void setTimerFrequencyPulseWidth(TIM_HandleTypeDef* pTim, uint16_t freq, uint16_t pulseWidth, uint32_t channel){

	//foo->bar = (*foo).bar
	//frequency of auto reload (pwm frequency) = FCLK/(PSC+1)/(ARR+1)

	// Turn timer off first
	pTim->Instance->CCR1 = 0;
	pTim->Instance->CCR2 = 0;

	if(pulseWidth != 0 && freq > 0){

		uint32_t autoReloadReg = 0, prescaler = 0;
		//set bounds
		//if(freq < 1)freq = 1;
		if(freq>MAX_FREQUENCY) freq = MAX_FREQUENCY;

		int minPulseWidth = 16/freq;	//for low frequencies. We only have an accuracy of 16us at 1 Hz
		if(minPulseWidth < 5)minPulseWidth = 5;

		if(pulseWidth > MAX_PULSE_WIDTH) pulseWidth = MAX_PULSE_WIDTH;
		if(pulseWidth < minPulseWidth) pulseWidth = minPulseWidth;

		//Precalculated prescalers. We could calculate these every function call,
		//but why waste the time.
		// autoReloadReg and prescaler need to fit into 16 bits
		if(freq == 1)        prescaler = 1024 - 1;
		else if(freq <= 3)   prescaler = 512-1;
		else if(freq <= 7)   prescaler = 256-1;
		else if(freq <= 15)  prescaler = 128-1;
		else if(freq <= 32)  prescaler = 64-1;
		else if(freq <= 63)  prescaler = 32-1;
		else if(freq <= 127) prescaler = 16-1;
		else if(freq <= 255) prescaler = 8-1;
		else if(freq <= 511) prescaler = 4-1;
		else prescaler = 2-1;

		//always round down (no need to -1, close enough)
		autoReloadReg = CPU_CLK_HZ / ((prescaler+1) * freq);

		//float usPerBit = ((float)(prescaler+1) / 32000000.0) * 1000000.0;
		//int bits = (int)((float)pulseWidth / usPerBit);
		float usPerBit = (float)(prescaler+1) / 64.0;
		uint32_t bits = (uint32_t)((float)pulseWidth / usPerBit);

		pTim->Instance->ARR = (uint32_t)autoReloadReg;
		pTim->Instance->PSC = (uint32_t)prescaler;
		if(channel == TIM_CHANNEL_1)     pTim->Instance->CCR1 = bits;
		else if(channel == TIM_CHANNEL_2)pTim->Instance->CCR2 = bits;
	}
	else{
		pTim->Instance->CCR1 = 0;
		pTim->Instance->CCR2 = 0;
		//		HAL_TIM_PWM_Stop(ptim, channel);
	}
	// Force update of PSC/ARR immediately
	pTim->Instance->EGR = TIM_EGR_UG;
}

// void stopTimer(TIM_HandleTypeDef* pTim, uint32_t channel){
// 	pTim->Instance->CCR1 = 0;
// 	HAL_TIM_PWM_Stop(pTim, channel);
// }
