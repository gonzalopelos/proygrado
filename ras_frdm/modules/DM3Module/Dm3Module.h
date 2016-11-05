#ifndef __task_dm3_h
#define __task_dm3_h

#include "mcc.h"
#include "rtos.h"

#define DM3_OPCODES 3
//#define OPCODE_REPORT (unsigned int)0
#define OPCODE_SIREN 1
#define OPCODE_BATTERY 2

class Dm3Module
{
	int pid;
	OpcodeCallback opcode_callbacks[DM3_OPCODES];

	void report_siren();

public:
	Dm3Module ();
	static void siren_task(void const *argument);
	static void battery_report_task(void const *argument);

};


#endif
