/*
 * midi_keyboard_handler.c
 *
 *  Created on: Aug 4, 2025
 *      Author: bucht
 */


#include "midi_keyboard_handler.h"

typedef enum {
    MIDI_RECV_STATUS    = 0,
    MIDI_RECV_DB1       = 1,
    MIDI_RECV_DB2       = 2,
    MIDI_WAIT_SYSEX_END = 3
} MidiRecvState;


MidiRecvState midi_recv_state = MIDI_RECV_STATUS;

uint8_t next_midi_byte;
MidiMsg_t next_midi_msg;

MidiMsg_t midi_msg_q[MIDI_MSG_Q_LEN];
uint16_t midi_q_head = 0;
uint16_t midi_q_tail = 0;
uint8_t num_in_midi_msg_q = 0;

// Flag to indicate if a message has been lost 
uint8_t midi_msg_lost = 0;

UART_HandleTypeDef* KEYBOARD_UART;

void midi_keyboard_init(UART_HandleTypeDef* handle){
	KEYBOARD_UART = handle;
}

UART_HandleTypeDef* get_keyboard_uart_ptr(){
	return KEYBOARD_UART;
}

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

void deq_next_midi_msg(MidiMsg_t* p_msg){
	__disable_irq();
	uint8_t midi_tail_idx = midi_q_tail % MIDI_MSG_Q_LEN;
	if(num_in_midi_msg_q == 0){
		p_msg = NULL;
	}
	else{
//		p_msg = &midi_msg_q[midi_tail_idx];
		p_msg->status = midi_msg_q[midi_tail_idx].status;
		p_msg->db1 = midi_msg_q[midi_tail_idx].db1;
		p_msg->db2 = midi_msg_q[midi_tail_idx].db2;
		midi_q_tail++;
		num_in_midi_msg_q--;
	}
	__enable_irq();
}

uint8_t num_pending_midi_msg(){
	return num_in_midi_msg_q;
}

void midi_start_rx_it(){
	HAL_UART_Receive_IT(KEYBOARD_UART, &next_midi_byte, 1);
}

void handle_midi_rx_it(){
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

uint8_t calc_midi_msg_tot_num_bytes(uint8_t status_byte){
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
