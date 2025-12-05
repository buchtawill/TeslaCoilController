/*
 * midi_keyboard_handler.c
 *
 *  Created on: Aug 4, 2025
 *      Author: bucht
 */


#include "midi_keyboard_handler.h"
#include "midi.h"

MidiRecvState midi_recv_state = MIDI_RECV_STATUS;

static uint8_t next_midi_byte;
static MidiMsg_t next_midi_msg;

static MidiMsg_t midi_msg_q[MIDI_MSG_Q_LEN];
static uint16_t midi_q_head = 0;
static uint16_t midi_q_tail = 0;
static uint8_t num_in_midi_msg_q = 0;

// Flag to indicate if a message has been lost 
static uint8_t midi_msg_lost = 0;

// This function is called within the UART ISR and hence cannot be overwritten
void midi_enq_and_reset_state(){
	midi_recv_state = MIDI_RECV_STATUS;

	if(num_in_midi_msg_q == MIDI_MSG_Q_LEN){
		// Oldest message is lost :(
		midi_q_tail++;
		midi_msg_lost++;
	}

	uint8_t midi_head_idx = midi_q_head % MIDI_MSG_Q_LEN;

	midi_msg_q[midi_head_idx] = next_midi_msg;
	midi_q_head++;
	num_in_midi_msg_q++;
}

MidiMsg_t* deq_next_midi_msg(){
	MidiMsg_t *retval = NULL;
	__disable_irq();
	uint8_t midi_tail_idx = midi_q_tail % MIDI_MSG_Q_LEN;
	if(num_in_midi_msg_q == 0){
		retval = NULL;
	}
	else{
		retval = &midi_msg_q[midi_tail_idx];
		midi_q_tail++;
		num_in_midi_msg_q--;
	}
	__enable_irq();
	return retval;
}

uint8_t num_pending_midi_msg(){
	return num_in_midi_msg_q;
}

void keyboard_start_rx_it(){
	HAL_UART_Receive_IT(&KEYBOARD_UART_HANDLE, &next_midi_byte, 1);
}

void handle_keyboard_rx_it(){
	// next_midi_byte has some new stuff in it at this point

	// For each 
	switch(midi_recv_state){
		case MIDI_RECV_STATUS:
			next_midi_msg.status = next_midi_byte;

			// If it's a sysex start message, just ignore all the data until it's over
			if(next_midi_byte == MIDI_MSG_SYSEX_START){
				midi_recv_state = MIDI_WAIT_SYSEX_END;
				break;
			}

			next_midi_msg.msg_num_bytes = calc_midi_msg_tot_num_bytes(next_midi_byte);
			if(next_midi_msg.msg_num_bytes > 1) midi_recv_state = MIDI_RECV_DB1;
			else midi_enq_and_reset_state();
		break;
		//////////////////////
		case MIDI_RECV_DB1:
			next_midi_msg.db1 = next_midi_byte;
			if(next_midi_msg.msg_num_bytes > 2) midi_recv_state = MIDI_RECV_DB2;
			else midi_enq_and_reset_state();
		break;
		//////////////////////
		case MIDI_RECV_DB2:
			next_midi_msg.db2 = next_midi_byte;
			midi_enq_and_reset_state();
		break;
		//////////////////////
		case MIDI_WAIT_SYSEX_END:
			if(next_midi_byte == MIDI_MSG_SYSEX_END) midi_recv_state = MIDI_RECV_STATUS;
		break;
	}
}
