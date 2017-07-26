//
// Created by Gonzalo Pelós on 2/8/17.
//

#ifndef DM3_CORE_MCC_H
#define DM3_CORE_MCC_H
#include "EmBencode.h"

#define MAX_POLL_CALLBACKS 50
#define MAX_PIDS 50
#define MAX_PARAMS (long)10
#define ADMIN_MODULE_PID  (long)0
#define MOTOR_MODULE_PID (long)1
#define DM3_MODULE_PID (long)2
#define OPCODE_REPORT (long)0


typedef void (*PollCallback) ();
typedef int (*OpcodeCallback) (unsigned int pid, unsigned int opcode, char* bc_params);
typedef OpcodeCallback* OpcodeCallbacks;

struct OpcodesCallbackReg {
    OpcodeCallbacks callbacks;
    uint8_t n_callbacks;
};

typedef OpcodesCallbackReg OpcodesCalbbackReg;

class Mcc_Controller {
    PollCallback poll_callbacks[MAX_POLL_CALLBACKS];
    int n_poll_callbacks;
    OpcodesCallbackReg opcode_callbacks[MAX_PIDS];
    uint8_t opcode_callbacks_count[MAX_PIDS]; //FIXME merge with previous as an array of structs.
    int n_opcode_callbacks;
    void send_parse_error_message();
    void send_execution_error_message(int tpid, int opcode, int errcode);
    char* create_mcc_params_from_incomming_data();
public:
    Mcc_Controller();
    virtual ~Mcc_Controller();
    char* incomming_params_s[MAX_PARAMS];
    int incomming_params_n[MAX_PARAMS];
    int incomming_params_count;
    EmBencode encoder;
    void process_incomming(char ch);
    int register_poll_callback(PollCallback cb);
    void unregister_poll_callback(int);
    int register_opcode_callbacks(OpcodeCallback* opcode_callbacks, uint8_t opcodes_count);
    void send_message(int spid, uint8_t opcode, char const *data, uint8_t data_length);
    // void tick();
    char * create_mcc_message(int spid, uint8_t opcode, char const * params_bencoded);
};
#endif //DM3_CORE_MCC_H
