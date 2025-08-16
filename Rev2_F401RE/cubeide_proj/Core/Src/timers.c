#include "timers.h"
#include "main.h"
#include "midi.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

typedef struct{
	TIM_HandleTypeDef *ptim;
	uint8_t note_num;
	uint16_t note_freq;
	uint8_t is_playing;
	uint32_t pwm_ch;
} midi_timer;

static midi_timer mt1;
static midi_timer mt2;

void init_timers(){
	mt1.is_playing = 0;
	mt1.ptim = &htim1;
	mt1.pwm_ch = TIM_CHANNEL_1;

	mt2.is_playing = 0;
	mt2.ptim = &htim2;
	mt2.pwm_ch = TIM_CHANNEL_1;
}

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

/**
 * Handle the given note-on message and assign to timers
 */
void handle_note_on(MidiMsg_t *msg){
	// Keyboard defaults to leftmost key being note 48 (C3)
	uint8_t note_num = msg->db1;

	// We only have 96 entries in the MIDI_NOTE_FREQ table
	if(note_num >= 96){
		return;
	}
	uint16_t freq = midi_note_to_freq(note_num);

	if(mt1.is_playing == 0){
		mt1.is_playing = 1;
		mt1.note_freq = freq;
		mt1.note_num = note_num;
		setTimerFrequencyPulseWidth(mt1.ptim, freq, 75, mt1.pwm_ch);
	}
	else if(mt2.is_playing == 0){
		mt2.is_playing = 1;
		mt2.note_freq = freq;
		mt2.note_num = note_num;
		setTimerFrequencyPulseWidth(mt2.ptim, freq, 75, mt2.pwm_ch);
	}
}

/**
 * Handle the given note-of message, deactivating timers
 */
void handle_note_off(MidiMsg_t *msg){
	uint8_t note_num = msg->db1;

	if(mt1.is_playing && (mt1.note_num == note_num)){
		mt1.is_playing = 0;
		setTimerFrequencyPulseWidth(mt1.ptim, 0, 0, mt1.pwm_ch);
	}
	else if(mt2.is_playing && (mt2.note_num == note_num)){
		mt2.is_playing = 0;
		setTimerFrequencyPulseWidth(mt2.ptim, 0, 0, mt2.pwm_ch);
	}
}

/**
 * Approximate pitch-bend messages by using a quadratic taylor series 
 * f = f0 * 2^(semitones/12)
 */
static inline uint16_t approximate_semitone_freq(float semitones, uint16_t base_freq) {
    // r = semitone offset in octaves (12 semitones = 1 octave)
    float r = semitones / 12.0f;

    // Quadratic approximation of 2^r
    const float LN2 = 0.69314718f;
    float factor = 1.0f + (LN2 * r) + (0.5f * LN2 * LN2 * r * r);

	float new_freq = (float)base_freq * factor;

    // Scale base frequency
    return (uint16_t)new_freq;
}


void handle_midi_output_msg(MidiMsg_t *msg){

	uint8_t status  = msg->status;
    // uint8_t channel = status & 0x0F;  // lower nibble = channel 0–15
    uint8_t type    = status & 0xF0;  // upper nibble = message type

	switch(type){
		case MIDI_MSG_NOTE_ON: handle_note_on(msg); break;
		/////////////////////////////////////////////////////////////////////////
		case MIDI_MSG_NOTE_OFF: handle_note_off(msg); break;
		/////////////////////////////////////////////////////////////////////////
		case MIDI_MSG_PITCH_BEND: {
            // Data1 = LSB, Data2 = MSB
            int16_t raw = (msg->db2 << 7) | msg->db1;
            raw -= 8192; // center to 0

			// Get the frequencies currently playing, and update
			// Semitone offset = raw/8192 * 2
			float semitones = ((float)raw) / 4096;

			if(mt1.is_playing) setTimerFrequencyPulseWidth(mt1.ptim,  approximate_semitone_freq(semitones, mt1.note_freq), 50, mt1.pwm_ch);
			if(mt2.is_playing) setTimerFrequencyPulseWidth(mt2.ptim,  approximate_semitone_freq(semitones, mt2.note_freq), 50, mt2.pwm_ch);

            break;
        }
		default: break;
	}
}


void shutoff_all_notes(){
	mt1.is_playing = 0;
	mt2.is_playing = 0;
	setTimerFrequencyPulseWidth(mt1.ptim, 0, 0, mt1.pwm_ch);
	setTimerFrequencyPulseWidth(mt2.ptim, 0, 0, mt2.pwm_ch);
}
