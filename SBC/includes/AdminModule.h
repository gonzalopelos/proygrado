//
// Created by Gonzalo Pelós on 2/8/17.
//

#ifndef SBC_ADMINMODULE_H
#define SBC_ADMINMODULE_H

#include "Mcc.h"

//#define MODULE_VERSION 1
//char admin_str[] = "admin";


#define ADMIN_OPCODES 3
//#define OPCODE_REPORT 0
#define OPCODE_PING 1
#define OPCODE_RESET 2


class AdminModule
{
    int pid;
    OpcodeCallback opcode_callbacks[ADMIN_OPCODES];
public:
    static int report_status(char * str, int len);
    AdminModule ();
};

#endif //SBC_ADMINMODULE_H
