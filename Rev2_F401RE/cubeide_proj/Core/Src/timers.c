#include "timers.h"
#include "main.h"
#include "midi.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
static uint8_t tim1_note = 255;
static uint8_t tim2_note = 255;

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
	// Clear all just in case
	pTim->Instance->CCR1 = 0;
	pTim->Instance->CCR2 = 0;
	pTim->Instance->CCR3 = 0;
	pTim->Instance->CCR4 = 0;

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
	// Force update of PSC/ARR immediately
	pTim->Instance->EGR = TIM_EGR_UG;
}


void handle_midi_output_msg(MidiMsg_t *msg){

	// Note on
	if((msg->status & 0xF0) == MIDI_MSG_NOTE_ON){

		// Keyboard defaults to leftmost key being note 48 (C3)
		uint8_t note_num = msg->db1;

		// We only have 96 entries in the MIDI_NOTE_FREQ table
		if(note_num >= 96){
			return;
		}
		uint16_t freq = midi_note_to_freq(note_num);

		if(tim1_note == 255){
			setTimerFrequencyPulseWidth(&htim1, freq, 75, TIM_CHANNEL_1);
			tim1_note = note_num;
		}
		else if(tim2_note == 255){
			setTimerFrequencyPulseWidth(&htim2, freq, 75, TIM_CHANNEL_1);
			tim2_note = note_num;
		}
	}

	// Note off
	else if((msg->status & 0xF0) == MIDI_MSG_NOTE_OFF){
		uint8_t note_num = msg->db1;
		if(tim1_note == note_num){
			setTimerFrequencyPulseWidth(&htim1, 0, 0, TIM_CHANNEL_1);
			tim1_note = 255; // off
		}
		else if(tim2_note == note_num){
			setTimerFrequencyPulseWidth(&htim2, 0, 0, TIM_CHANNEL_1);
			tim2_note = 255; // off
		}
	}
}


void shutoff_all_notes(){
	tim1_note = 255; // off
	tim2_note = 255; // off
	setTimerFrequencyPulseWidth(&htim1, 0, 0, TIM_CHANNEL_1);
	setTimerFrequencyPulseWidth(&htim2, 0, 0, TIM_CHANNEL_1);
}