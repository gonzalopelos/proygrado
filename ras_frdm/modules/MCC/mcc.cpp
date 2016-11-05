/*
 * mcc.cpp
 *
 *  Created on: Oct 4, 2013
 *      Author: jvisca
 */

#include "mbed.h"
#include "mcc.h"

Serial pc(USBTX, USBRX);
Mutex serial_mcc_frame_lock;

DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);


//int n_bytes = 0;
//LocalFileSystem local("local");               // Create the local filesystem under the name "local"
//FILE *fp = fopen("/local/out.txt", "w");  // Open "out.txt" on the local file system for writing


enum ProtocolStates {
	MESSAGE, TPID, OPCODE, DATA, DATA_ITEM, END, NONE
};

enum ProtocolStates protocol_state = MESSAGE;

char embuf[255];
EmBdecode decoder(embuf, sizeof embuf);

uint8_t tpid;
uint8_t opcode;


void EmBencode::lock() {
	serial_mcc_frame_lock.lock();
}

void EmBencode::unlock() {
	serial_mcc_frame_lock.unlock();
}

void EmBencode::PushChar(char ch) {
	////////Serial.write(ch);
	led4 = 1;
	while (pc.writeable()==0) {}
	pc.putc(ch);
	led4 = 0;
}

MCC::MCC() {
	n_poll_callbacks = 0;
	n_opcode_callbacks = 0;
	//led2 = false;
	Serial pc(USBTX, USBRX);
	pc.baud(9600); // no le da pelota a esto

	//MCC::send_message(0, EVCODE_REPORT, "STARTED", 7);
}

int MCC::register_poll_callback(PollCallback cb) {
	if (MCC::n_poll_callbacks >= MAX_POLL_CALLBACKS) {
		return -1;
	}
	MCC::poll_callbacks[MCC::n_poll_callbacks++] = cb;
	return MCC::n_poll_callbacks;
}

void MCC::unregister_poll_callback(int n) {
	if (n<MCC::n_poll_callbacks-1){
		MCC::poll_callbacks[n] = MCC::poll_callbacks[MCC::n_poll_callbacks-1];
	}
	--MCC::n_poll_callbacks;
}

void MCC::tick() {
	MCC::process_incomming();
	for (int i=0; i<MCC::n_poll_callbacks; ++i){
		MCC::poll_callbacks[i]();
	}
}

int MCC::register_opcode_callbacks(OpcodeCallbacks opcode_callbacks, uint8_t opcodes_count) {
	uint8_t pid = MCC::n_opcode_callbacks++;
	MCC::opcode_callbacks[pid].callbacks = opcode_callbacks;
	MCC::opcode_callbacks[pid].n_callbacks = opcodes_count;
	return pid;
}
//void unregister_opcode_callbacks(int pid);

/*
void MCC::process_echo() {
	if (pc.readable()) {
		pc.putc(pc.getc());
	}
}
*/

void MCC::send_message(int spid, uint8_t opcode, char const *data, uint8_t data_length) {
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

void MCC::send_parse_error_message() {
	//decoder.reset();

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

void MCC::send_execution_error_message(int tpid, int opcode, int errcode) {
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


void MCC::process_incomming() {
	if (pc.readable()) {
		char ch = pc.getc();

		if (ch=='\n') {
			decoder.reset();
			tpid = opcode = -1;
			incomming_params_count = 0;
			return;
		}

		//pc.putc('\0');
		//pc.putc(ch);
		//n_bytes++;
	    //fprintf(fp, "%c", ch);
	    //if (n_bytes==14000) {fclose(fp);}

		//if (n_bytes%100==0) {pc.printf('%d', n_bytes);}
		//pc.putc(ch);
		led3 = 1;
		uint8_t bytes = decoder.process(ch);
		if (bytes > 0) {
			led3 = 0;
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
						/*
						pc.putc('Z');
						pc.putc(token);
						pc.putc('z');
						protocol_state = MESSAGE;
						*/
						send_parse_error_message();
						protocol_state = NONE;
					}
					break;
				case END:
					if (token == EmBdecode::T_POP) {
						//pc.putc('?');
						//pc.putc(tpid);
						//pc.putc(opcode);
						//pc.putc(n_opcode_callbacks);
						//pc.putc((MCC::opcode_callbacks[tpid]).n_callbacks);
						if (tpid < MAX_PIDS
						&& tpid < MCC::n_opcode_callbacks
						&& opcode < (MCC::opcode_callbacks[tpid]).n_callbacks) {
							led2 = 1;
							//pc.putc('!');
							int ret = MCC::opcode_callbacks[tpid].callbacks[opcode](tpid, opcode);
							led2 = 0;
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
