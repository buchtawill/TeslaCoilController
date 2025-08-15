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
#include "midi.h"

/**
 * @brief return the number of bytes waiting in the UART buffer
 * (uart, not midi recv buf)
 */
uint16_t keyboard_pending_bytes();

/**
 * @brief wrapper around starting HAL_UART_Start IT
 */
void keyboard_start_rx_it();

/**
 * Push the byte into the next midi message, enqueue it when it's time.
 * This function is a state machine for handling midi messages
 */
void handle_keyboard_rx_it();

/**
 * @brief enqueue the current tmp midi message and reset the recv state
 */
void midi_enq_and_reset_state();

/**
 * @brief return the number of pending midi messages in the queue
 */
uint8_t num_pending_midi_msg();

/**
 * Get the next pending midi message, if there's one available
 * @return NULL if no messages available
 */
MidiMsg_t* deq_next_midi_msg();

#endif /* INC_MIDI_KEYBOARD_HANDLER_H_ */
