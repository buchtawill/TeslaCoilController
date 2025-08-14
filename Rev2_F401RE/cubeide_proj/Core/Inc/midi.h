/*
 * midi.h
 *
 *  Created on: Aug 10, 2025
 *      Author: bucht
 */

#ifndef MIDI_H
#define MIDI_H

#include <stdint.h>
#include <stddef.h> // for size_t

//------------------------------------------
// MidiMsg helpers
//------------------------------------------
typedef enum {
    MIDI_RECV_STATUS    = 0,
    MIDI_RECV_DB1       = 1,
    MIDI_RECV_DB2       = 2,
    MIDI_WAIT_SYSEX_END = 3
} MidiRecvState;

typedef enum {
    MIDI_FROM_SD       = 0,
    MIDI_FROM_KEYBOARD = 1
} MidiSrcId;

typedef struct MidiMsg{
	uint8_t status;
	uint8_t db1;
	uint8_t db2;
    uint8_t msg_num_bytes;
    MidiSrcId src_id;
} MidiMsg_t;

#define MIDI_MSG_Q_LEN     8 // max num pending MidiMsg_t
#define MIDI_MSG_Q_SIZE    (MIDI_MSG_Q_LEN*sizeof(MidiMsg_t))

//------------------------------------------
// MIDI Message types
//------------------------------------------

// Channel Voice Messages (0x8x to 0xEx)
#define MIDI_MSG_NOTE_OFF       0b10000000
#define MIDI_MSG_NOTE_ON        0b10010000
#define MIDI_MSG_PPHN_PRESSURE  0b10100000
#define MIDI_MSG_CTRL_CHANGE    0b10110000
#define MIDI_MSG_PRGM_CHANGE    0b11000000
#define MIDI_MSG_CHN_PRESSURE   0b11010000
#define MIDI_MSG_PITCH_BEND     0b11100000

// System Common Messages (0xF0 to 0xF7)
#define MIDI_MSG_SYSEX_START     0xF0  // System Exclusive Start
#define MIDI_MSG_TIME_CODE       0xF1  // MIDI Time Code Quarter Frame
#define MIDI_MSG_SONG_POSITION   0xF2  // Song Position Pointer
#define MIDI_MSG_SONG_SELECT     0xF3  // Song Select
#define MIDI_MSG_UNUSED_F4       0xF4  // Undefined
#define MIDI_MSG_UNUSED_F5       0xF5  // Undefined
#define MIDI_MSG_TUNE_REQUEST    0xF6  // Tune Request
#define MIDI_MSG_SYSEX_END       0xF7  // End of SysEx

// System Real-Time Messages (0xF8 to 0xFF)
#define MIDI_MSG_TIMING_CLOCK    0xF8
#define MIDI_MSG_UNUSED_F9       0xF9  // Undefined
#define MIDI_MSG_START           0xFA
#define MIDI_MSG_CONTINUE        0xFB
#define MIDI_MSG_STOP            0xFC
#define MIDI_MSG_UNUSED_FD       0xFD  // Undefined
#define MIDI_MSG_ACTIVE_SENSE    0xFE
#define MIDI_MSG_RESET           0xFF


/**
 * @brief Calculate the TOTAL number of bytes in the midi message,
 * including status byte
 */
static inline uint8_t calc_midi_msg_tot_num_bytes(uint8_t status_byte){
	uint8_t upper_only = status_byte & 0xF0;

	switch(upper_only){
		case MIDI_MSG_NOTE_OFF:
		case MIDI_MSG_NOTE_ON:
		case MIDI_MSG_PPHN_PRESSURE:
		case MIDI_MSG_CTRL_CHANGE:
		case MIDI_MSG_PITCH_BEND:
			return 3;

		case MIDI_MSG_PRGM_CHANGE:
		case MIDI_MSG_CHN_PRESSURE:
			return 2;
	}

	// System Common and Real-Time Messages (exact match needed)
    switch (status_byte) {
        case MIDI_MSG_TIME_CODE:
        case MIDI_MSG_SONG_SELECT:
            return 2;

        case MIDI_MSG_SONG_POSITION:
            return 3;

        case MIDI_MSG_TUNE_REQUEST:
        case MIDI_MSG_SYSEX_END:
        case MIDI_MSG_TIMING_CLOCK:
        case MIDI_MSG_START:
        case MIDI_MSG_CONTINUE:
        case MIDI_MSG_STOP:
        case MIDI_MSG_ACTIVE_SENSE:
        case MIDI_MSG_RESET:
            return 1;

        case MIDI_MSG_SYSEX_START:
            return 0xFF;  // Special case: variable length (not supported here)

        case MIDI_MSG_UNUSED_F4:
        case MIDI_MSG_UNUSED_F5:
        case MIDI_MSG_UNUSED_F9:
        case MIDI_MSG_UNUSED_FD:
        default:
            return 0;  // Invalid or unsupported message
    }
}


#endif
