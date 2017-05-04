/*
 * Dm3Module.h
 *
 *  Created on: May 2, 2017
 *      Author: gonzalopelos
 */

#ifndef MODULES_DM3_DM3MODULE_H_
#define MODULES_DM3_DM3MODULE_H_

#include "../Mcc/Mcc.h"



namespace modules {

//#define OPCODE_REPORT 0
#define OPCODE_SIREN 1
#define OPCODE_BATTERY 2
#define OPCODE_ULTRASONICS_REPORT 3
#define DM3_OPCODES 4

class Dm3Module {
private:
	int pid;
	OpcodeCallback opcode_callbacks[DM3_OPCODES];
	void report_siren();

public:
	Dm3Module();
	virtual ~Dm3Module();
	static void siren_task(void const *argument);
	static void battery_report_task(void const *argument);
};

} /* namespace modules */

#endif /* MODULES_DM3_DM3MODULE_H_ */
