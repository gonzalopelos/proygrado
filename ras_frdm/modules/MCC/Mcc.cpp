/*
 * Mcc.cpp
 *
 *  Created on: Jan 21, 2017
 *      Author: gonzalopelos
 */

#include "Mcc.h"

Mcc::Mcc() {
	// TODO Auto-generated constructor stub

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

	return result;
}

void Mcc::unregister_poll_callback(int int1) {
}

int Mcc::register_opcode_callbacks(OpcodeCallback* opcode_callbacks, uint8_t opcodes_count) {
	int result = 0;

	return result;
}

void Mcc::send_message(int spid, uint8_t opcode, const char* data, uint8_t data_length) {
}

void Mcc::tick() {
}
