#ifndef __TIMERS_H__
#define __TIMERS_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "stm32f4xx_hal.h"
#include "main.h"
#include "midi.h"

#define CPU_CLK_HZ          64000000
#define MAX_PULSE_WIDTH     150 //microseconds
#define MIN_PULSE_WIDTH     10  //microseconds
#define MAX_FREQUENCY      		1180 // Hz: D6 = 1174
#define ONTIME_NERFING_BOUND	800  // Upper bound of no restriction for ontime. Above this, divide on time by 2
#define DUTY_CYCLE_LIMIT		10   // 10% limit on duty cycle
#define MAX_TIME_ON      		500  // ms, for burst mode
#define MAX_TIME_OFF  			500  // ms, for burst mode

/**
 * @brief 	sets a given timer to a frequency of freq (Hz) and pulseWidth (us). Set pulsewidth to 0 to turn off
 * @param	pTim pointer to the timer struct
 * @param	freq frequency in hertz
 * @param	pulseWidth desired pulsewidth in microseconds. Be careful of low frequencies!
 * @param channel the timer channel of pTim to apply this to
 * @param pre_shutoff whether or not to pre-zero CCRx
 */
void setTimerFrequencyPulseWidth(TIM_HandleTypeDef* pTim, uint16_t freq, uint16_t pulseWidth, uint32_t channel);
void startTimer(TIM_HandleTypeDef* pTim, uint32_t channel);
void stopTimer(TIM_HandleTypeDef* pTim, uint32_t channel);

/**
 * @brief handle MIDI messages that are meant to produce sound. Keep track of which timers are doing which note.
 * This function has no concept of which coil is which and will assume that channel 0 corresponds to 2 timers, channel 1
 * corresponds to 2 other timers, etc
 *
 * How to use:
 * 	1. Call with a midi message
 * 	2. if the message was processed immediately, return the next message to process. Else null.
 *
 * 	if a message is enqueued, return null (no action necessary)
 * 	if a message is processed immediately and Q not empty, return ptr
 * 	else null
 *
 */
MidiMsg_t* handle_midi_output_msg(MidiMsg_t *msg);

/**
 * @brief Turn off all PWM and reset the state of the note tracker
 */
void shutoff_all_notes();

/**
 * Do any initialization needed for timers.c
 */
void init_timers();

// Critical section safe to get millis cnt
extern volatile uint64_t _millis;
static inline uint64_t get_millis(){
	uint64_t t1, t2;
	do {
		t1 = _millis;
		t2 = _millis;
	} while (t1 != t2);
	return t1;
}

static inline void inc_millis(){
    _millis++;
}

#ifdef __cplusplus
}
#endif
#endif //#ifndef __TIMERS_H__
