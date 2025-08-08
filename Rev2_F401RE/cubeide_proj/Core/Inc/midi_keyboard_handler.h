/*
 * midi_keyboard_handler.h
 *
 *  Created on: Aug 4, 2025
 *      Author: bucht
 *
 * This header encompasses all functionality needed
 */

#ifndef MIDI_KEYBOARD_HANDLER_H_
#define MIDI_KEYBOARD_HANDLER_H_

#include "main.h"

// Channel Voice Messages (0x8n to 0xEn)
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

typedef struct MidiMsg{
	uint8_t status;
	uint8_t db1;
	uint8_t db2;
    uint8_t msg_num_bytes;
} MidiMsg_t;

#define MIDI_MSG_Q_LEN     64 // max num pending MidiMsg_t
#define MIDI_MSG_Q_SIZE    (MIDI_MSG_Q_LEN*sizeof(MidiMsg_t))

void midi_keyboard_init(UART_HandleTypeDef* handle);

/**
 * @brief Get the uart config pointer that the keyboard uses
 */
UART_HandleTypeDef* get_keyboard_uart_ptr();

/**
 * @brief Calculate the TOTAL number of bytes in the midi message
 */
uint8_t calc_midi_msg_tot_num_bytes(uint8_t status_byte);

/**
 * @brief return the number of bytes waiting in the UART buffer
 * (uart, not midi recv buf)
 */
uint16_t keyboard_pending_bytes();

/**
 * @brief wrapper around starting HAL_UART_Start IT
 */
void midi_start_rx_it();

/**
 * Push the byte into the next midi message, enqueue it when it's time.
 * This function is a state machine for handling midi messages
 */
void handle_midi_rx_it();

/**
 * @brief enqueue the current tmp midi message and reset the recv state
 */
void midi_enq_and_reset_state();

/**
 * @brief return the number of pending midi messages in the queue
 */
uint8_t num_pending_midi_msg();

/**
 * Get the next pending midi message
 */
void deq_next_midi_msg(MidiMsg_t* p_msg);

#endif /* INC_MIDI_KEYBOARD_HANDLER_H_ */
