/*
 * Mcc.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: gonzalopelos
 */

#include "Mcc.h"
#include "../Ethernet/Communication.h"

enum ProtocolStates {
	MESSAGE, TPID, OPCODE, DATA, DATA_ITEM, END, NONE
};

enum ProtocolStates protocol_state = MESSAGE;
char embuf[255];

EmBdecode decoder(embuf, sizeof embuf);
Communication * host = Communication::get_instance();
uint8_t tpid;
uint8_t opcode;



Mcc::Mcc() {
	incomming_params_count = 0;
	n_poll_callbacks = 0;
	n_opcode_callbacks = 0;
}

Mcc::~Mcc() {
	// TODO Auto-generated destructor stub
}

void Mcc::process_incomming() {
}

void Mcc::send_parse_error_message() {
}

void Mcc::send_execution_error_message(int tpid, int opcode, int errcode) {
}

int Mcc::register_poll_callback(PollCallback cb) {
	int result = 0;

	if (Mcc::n_poll_callbacks >= MAX_POLL_CALLBACKS) {
		return -1;
	}
	Mcc::poll_callbacks[Mcc::n_poll_callbacks++] = cb;
	return Mcc::n_poll_callbacks;

	return result;
}

void Mcc::unregister_poll_callback(int n) {
	if (n<Mcc::n_poll_callbacks-1){
		Mcc::poll_callbacks[n] = Mcc::poll_callbacks[Mcc::n_poll_callbacks-1];
	}
}

int Mcc::register_opcode_callbacks(OpcodeCallback* opcode_callbacks, uint8_t opcodes_count) {
	int result = -1;
	if(Mcc::n_opcode_callbacks < MAX_PIDS){
		uint8_t pid = Mcc::n_opcode_callbacks++;
		Mcc::opcode_callbacks[pid].callbacks = opcode_callbacks;
		Mcc::opcode_callbacks[pid].n_callbacks = opcodes_count;
		result = pid;
	}
	return result;
}

void Mcc::send_message(int spid, uint8_t opcode, const char* data, uint8_t data_length) {
}

void Mcc::tick() {
}
