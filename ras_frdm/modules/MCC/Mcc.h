/*
 * Mcc.h
 *
 *  Created on: Jan 21, 2017
 *      Author: gonzalopelos
 */

#ifndef MODULES_MCC_MCC_H_
#define MODULES_MCC_MCC_H_

#include "../EmBencode/EmBencode.h"

#define MAX_POLL_CALLBACKS 50
#define MAX_PIDS 50
#define MAX_PARAMS 10
#define ADMIN_PID (long int) 0
#define MOTOR_PID (long int) 1
#define DM3_PID (long int) 2
#define OPCODE_REPORT (long int)0

char ok_str[] = "OK";
char error_str[] = "ERROR";
char parseerror_str[] = "PARSEERROR";
char exeerror_str[] = "EXEERROR";


typedef void (*PollCallback) ();
typedef int (*OpcodeCallback) (unsigned int pid, unsigned int opcode);
typedef OpcodeCallback* OpcodeCallbacks;

struct OpcodesCallbackReg {
	OpcodeCallbacks callbacks;
	uint8_t n_callbacks;
};

typedef OpcodesCallbackReg OpcodesCalbbackReg;

class Mcc {
	PollCallback poll_callbacks[MAX_POLL_CALLBACKS];
	int n_poll_callbacks;
	OpcodesCallbackReg opcode_callbacks[MAX_PIDS];
	//uint8_t opcode_callbacks_count[MAX_PIDS]; //FIXME merge with previous as an array of structs.
	int n_opcode_callbacks;
    void process_incomming();
    void send_parse_error_message();
    void send_execution_error_message(int tpid, int opcode, int errcode);
	EmBencode encoder;
	char* incomming_params_s[MAX_PARAMS];
	int incomming_params_n[MAX_PARAMS];
	int incomming_params_count = 0;
public:
	Mcc();
	virtual ~Mcc();
	int register_poll_callback(PollCallback cb);
	void unregister_poll_callback(int);
	int register_opcode_callbacks(OpcodeCallback* opcode_callbacks, uint8_t opcodes_count);
    void send_message(int spid, uint8_t opcode, char const *data, uint8_t data_length);
	void tick();
};


#endif /* MODULES_MCC_MCC_H_ */
