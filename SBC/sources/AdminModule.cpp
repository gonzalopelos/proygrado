//
// Created by Gonzalo Pelós on 2/8/17.
//

#include "../includes/AdminModule.h"

#include <unistd.h>


extern Mcc mcc;

//char admin_str[] = {'a','d','m','i','n'};

static int handle_report(unsigned int  pid, unsigned int  opcode) {
    int result = 0;


    return result;
}
static int handle_ping(unsigned int  pid, unsigned int  opcode) {
    //mcc.send_message(pid, OPCODE_PING, data, data_length);

    mcc.encoder.startFrame();
    mcc.encoder.push((long int)ADMIN_PID);
    mcc.encoder.push(OPCODE_PING);
    mcc.encoder.startList();
    for (int i=0;i<mcc.incomming_params_count;++i) {
        if (mcc.incomming_params_s[i] != NULL) {
            mcc.encoder.push(mcc.incomming_params_s[i], mcc.incomming_params_n[i]);
            printf("Mcc :: incomming_params_s received: %s\n", mcc.incomming_params_s[i]);
        } else {
            mcc.encoder.push(mcc.incomming_params_n[i]);
        }
    }
    mcc.encoder.endList();
    mcc.encoder.endFrame();
    return 1;
}

static int handle_reset(unsigned int  pid, unsigned int opcode) {
    mcc.encoder.startFrame();
    mcc.encoder.push(ADMIN_PID);
    mcc.encoder.push(OPCODE_RESET);
    mcc.encoder.startList();
    mcc.encoder.push("RESET", 5);
    mcc.encoder.endList();
    mcc.encoder.endFrame();
    usleep(1000); // espera a que se mande el mensaje

    return 1;
}

int AdminModule::report_status(char * str, int len) {
    int result = 0;


    return result;
}

AdminModule::AdminModule() {
    for (int i=0; i<ADMIN_OPCODES; ++i) {
        AdminModule::opcode_callbacks[i]=NULL;
    }
    AdminModule::opcode_callbacks[OPCODE_REPORT] = &handle_report;
    AdminModule::opcode_callbacks[OPCODE_PING] = &handle_ping;
    AdminModule::opcode_callbacks[OPCODE_RESET] = &handle_reset;
    AdminModule::pid = mcc.register_opcode_callbacks(AdminModule::opcode_callbacks, ADMIN_OPCODES);
}
