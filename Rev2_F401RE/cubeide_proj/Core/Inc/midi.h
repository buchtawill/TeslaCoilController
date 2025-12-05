/*
 * midi.h
 *
 *  Created on: Aug 10, 2025
 *      Author: bucht
 */

#ifndef MIDI_H
#define MIDI_H
#ifdef __cplusplus
 extern "C" {
#endif
#include <stdint.h>
#include <stddef.h> // for size_t

static uint16_t MIDI_NOTE_FREQ[96] = {
		//c,      c#,     d,      d#,     e,      f,      f#,     G,      g#,     a,      a#,     b
		 8,       9,      9,      10,     10,     11,     12,     12,     13,     14,     15,     15,  // MIDI only
		 16,      17,     18,     19,     21,     22,     23,     25,     26,     28,     29,     31,  // 0's
		 33,      35,     37,     39,     41,     44,     46,     49,     52,     55,     58,     61,  // 1's
		 65,      69,     73,     78,     82,     87,     93,     98,     104,    110,    117,    123, // 2's
		 131,     139,    147,    156,    165,    175,    185,    196,    208,    220,    233,    247, // 3's
		 262,     277,    294,    311,    330,    349,    370,    392,    415,    440,    466,    494, // 4's
		 523,     554,    587,    622,    659,    698,    740,    784,    831,    880,    932,    988, // 5's
		 1047,    1109,   1175,  1245,   1319,   1397,   1480,   1568,   1661,   1760,   1865,   1976  // 6's
};

static inline uint16_t midi_note_to_freq(uint8_t note_num){
    if(note_num >= 96){
        return 0;
    }
    else return MIDI_NOTE_FREQ[note_num];
}

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

#define MIDI_MSG_Q_LEN     16 // max num pending MidiMsg_t
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

/**
 * Copy all fields of src into dst
 */
static inline void midi_msg_cpy(MidiMsg_t *dst, MidiMsg_t *src){
    dst->db1 = src->db1;
    dst->db2 = src->db2;
    dst->msg_num_bytes = src->msg_num_bytes;
    dst->status = src->status;
}

#ifdef __cplusplus
}
#endif
#endif
