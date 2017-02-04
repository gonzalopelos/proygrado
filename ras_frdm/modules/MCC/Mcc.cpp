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
char outbuf[255];
EmBdecode decoder(embuf, sizeof embuf);
Communication * host = Communication::get_instance();
uint8_t tpid;
uint8_t opcode;
Mutex serial_mcc_frame_lock;

/***********************
 * EmBencode Operations
 ***********************/
void EmBencode::lock() {
	serial_mcc_frame_lock.lock();
}

void EmBencode::unlock() {
	serial_mcc_frame_lock.unlock();
}

void EmBencode::PushChar(char ch) {
	////////Serial.write(ch);
	/*led4 = 1;
	while (pc.writeable()==0) {}
	pc.putc(ch);
	led4 = 0;*/
	//ToDo: send tcp message.
	if(strlen(outbuf) < 255){
		strcat(outbuf, &ch);
		if(ch == '\n'){
			host->send_all(outbuf, strlen(outbuf));
			bzero(outbuf, 255);
		}
	}

}

/**************************/

Mcc::Mcc() {
	incomming_params_count = 0;
	n_poll_callbacks = 0;
	n_opcode_callbacks = 0;
	bzero(outbuf, 255);
}

Mcc::~Mcc() {
	// TODO Auto-generated destructor stub
}

void Mcc::process_incomming() {
	//ToDo: Get data from TCP and process by char
	char data[256];
	char ch;
	int data_length = 0;

	data_length = host->receive(data, 1);
	if(data_length > 0) {
		ch = data[0];
		for(int data_index =0;data_index<data_length;data_index++) {
			if (data[0]=='\n') {
				decoder.reset();
				tpid = opcode = -1;
				incomming_params_count = 0;
				return;
			}

			uint8_t bytes = decoder.process(ch);
			if (bytes > 0) {
				protocol_state = MESSAGE;
				for (;;) {
					uint8_t token = decoder.nextToken();
					if (token == EmBdecode::T_END) {
						//decoder.reset();
						break;
					}

					switch (protocol_state) {
					case MESSAGE:
						if (token == EmBdecode::T_LIST) {
							//pc.putc('A');
							protocol_state = TPID;
						} else {
							//pc.putc('X');
							send_parse_error_message();
							protocol_state = NONE;
						}
						break;
					case TPID:
						if (token == EmBdecode::T_NUMBER) {
							//pc.('B');
							tpid = decoder.asNumber();
							protocol_state = OPCODE;
						} else {
							//pc.putc('X');
							send_parse_error_message();
							protocol_state = NONE;
						}
						break;
					case OPCODE:
						if (token == EmBdecode::T_NUMBER) {
							//pc.putc('C');
							opcode = decoder.asNumber();
							protocol_state = DATA;
						} else {
							//pc.putc('X');
							send_parse_error_message();
							protocol_state = NONE;
						}
						break;
					case DATA:
						if (token == EmBdecode::T_LIST) {
							//pc.putc('I');
							protocol_state = DATA_ITEM;
							incomming_params_count = 0;
						} else {
							//pc.putc('X');
							send_parse_error_message();
							protocol_state = NONE;
						}
						break;
					case DATA_ITEM:
						if (token == EmBdecode::T_STRING) {
							//pc.putc('S');
							uint8_t data_length;
							incomming_params_s[incomming_params_count] = decoder.asString(&data_length);
							incomming_params_n[incomming_params_count] = data_length;
							incomming_params_count++;
							if (incomming_params_count==MAX_PARAMS) {
								//pc.putc('X');
								send_parse_error_message();
								protocol_state = NONE;
							}
						} else if (token == EmBdecode::T_NUMBER) {
							//pc.putc('N');
							incomming_params_s[incomming_params_count] = NULL;
							incomming_params_n[incomming_params_count] = decoder.asNumber();
							incomming_params_count++;
							if (incomming_params_count==MAX_PARAMS) {
								//pc.putc('Y');
								send_parse_error_message();
								protocol_state = NONE;
							}
						} else if (token == EmBdecode::T_POP) {
							//pc.putc('H');
							protocol_state = END;
						} else {
							send_parse_error_message();
							protocol_state = NONE;
						}
						break;
					case END:
						if (token == EmBdecode::T_POP) {
							if (tpid < MAX_PIDS
							&& tpid < Mcc::n_opcode_callbacks
							&& opcode < (Mcc::opcode_callbacks[tpid]).n_callbacks) {
								//pc.putc('!');
								int ret = Mcc::opcode_callbacks[tpid].callbacks[opcode](tpid, opcode);
								if (ret!=1) {
									send_execution_error_message(tpid, opcode, ret);
									//protocol_state = NONE;
								}
							}
							//decoder.reset();
							protocol_state = NONE;
							//pc.putc('E');
						} else {
							//pc.putc('W');
							send_parse_error_message();
							protocol_state = NONE;
						}
						break;
					case NONE:
						break;
					}
				}
				tpid = opcode = -1;
				incomming_params_count = 0;

				decoder.reset();
			}
		}
	}
}

void Mcc::send_parse_error_message() {
	encoder.startFrame();
	encoder.push(ADMIN_PID);
	encoder.push(OPCODE_REPORT);
	encoder.startList();
	encoder.push(parseerror_str, sizeof(parseerror_str)-1);

	encoder.push(tpid);
	encoder.push(opcode);
	encoder.push(protocol_state);

	for (int i=0;i<incomming_params_count;++i) {
		if (incomming_params_s[i] != NULL) {
			encoder.push(incomming_params_s[i], incomming_params_n[i]);
		} else {
			encoder.push(incomming_params_n[i]);
		}
	}

	encoder.endList();
	encoder.endFrame();
}

void Mcc::send_execution_error_message(int tpid, int opcode, int errcode) {
	encoder.startFrame();
	encoder.push(ADMIN_PID);
	encoder.push(OPCODE_REPORT);
	encoder.startList();
	encoder.push(exeerror_str, sizeof(exeerror_str)-1);
	encoder.push(errcode);
	encoder.push(tpid);
	encoder.push(opcode);
	for (int i=0;i<incomming_params_count;++i) {
		if (incomming_params_s[i] != NULL) {
			encoder.push(incomming_params_s[i], incomming_params_n[i]);
		} else {
			encoder.push(incomming_params_n[i]);
		}
	}
	encoder.endList();
	encoder.endFrame();
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
	encoder.startFrame();
	encoder.push(spid);
	encoder.push(opcode);
	encoder.startList();
	if (data!=NULL) {
		encoder.push(data, data_length);
	}
	encoder.endList();
	encoder.endFrame();
}

void Mcc::tick() {
	Mcc::process_incomming();
	for (int i=0; i<Mcc::n_poll_callbacks; ++i){
		Mcc::poll_callbacks[i]();
	}
}
