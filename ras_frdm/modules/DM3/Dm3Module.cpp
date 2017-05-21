/*
 * Dm3Module.cpp
 *
 *  Created on: May 2, 2017
 *      Author: gonzalopelos
 */

#include <Dm3Module.h>
#include "Dm3.h"
#include "../EmBencode/EmBencode.h"
#include "rtos.h"

using namespace modules;

extern Mcc mcc;


Dm3 * dm3_instance;


#define BATT_SENSE_PERIOD 1000*4
#define STRING_BUFF_SIZE 40
char stringbuffer[STRING_BUFF_SIZE];

int siren_count = 0;
int siren_on = 0;

Dm3Module::~Dm3Module() {
	// TODO Auto-generated destructor stub
}


static int handle_siren(unsigned int  pid, unsigned int  opcode) {
	siren_count = mcc.incomming_params_n[0] * 2; // la multiplicacion es porque el siren_task espera medio segundo
	return 1;
}

void Dm3Module::siren_task(void const *argument) {
    while (true) {
        if (siren_count==0) {
    		siren_on = 0;
    		dm3_instance->horn(siren_on);
    	} else {
    		if (siren_count>0) siren_count--;
    		siren_on = 1-siren_on;
    		dm3_instance->horn(siren_on);
    		mcc.encoder.startFrame();
    		mcc.encoder.push(DM3_PID);
    		mcc.encoder.push(OPCODE_SIREN);
    		mcc.encoder.startList();
    		mcc.encoder.push(siren_count);
    		mcc.encoder.endList();
    		mcc.encoder.endFrame();
    	}
        Thread::wait(500);
    }
}


static int handle_report(unsigned int  pid, unsigned int  opcode) {
	mcc.encoder.startFrame();
	mcc.encoder.push(DM3_PID);
	mcc.encoder.push(OPCODE_REPORT);
	mcc.encoder.startList();
	//mcc.encoder.push(admin_str, sizeof(admin_str)-1);
	//mcc.encoder.push(MODULE_VERSION);
	mcc.encoder.push(ok_str, sizeof(ok_str)-1);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
	return 1;
}


static int handle_batterylevel(unsigned int  pid, unsigned int  opcode) {
	mcc.encoder.startFrame();
	mcc.encoder.push(DM3_PID);
	mcc.encoder.push(OPCODE_BATTERY);
	mcc.encoder.startList();
	int len = snprintf(stringbuffer, STRING_BUFF_SIZE, "%.2f", 100.0*dm3_instance->get_batt());
	mcc.encoder.push(stringbuffer, len);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
	return 1;
}

//static int report_ultrasonic_sensors(unsigned int  pid, unsigned int  opcode){
//	mcc.encoder.startFrame();
//	mcc.encoder.push(DM3_PID);
//	mcc.encoder.push(OPCODE_ULTRASONICS_REPORT);
//	mcc.encoder.startList();
//	int len = snprintf(stringbuffer, STRING_BUFF_SIZE, "distances: [%d, %d, %d, %d]", dm3_instance->check_front_distances(), 0, 0, 0);
//	mcc.encoder.push(stringbuffer, len);
//	mcc.encoder.endList();
//	mcc.encoder.endFrame();
//	return 1;
//}

void Dm3Module::battery_report_task(void const *argument) {
	while(true){
		handle_batterylevel(DM3_PID, OPCODE_BATTERY);
		Thread::wait(BATT_SENSE_PERIOD);
	}

}

Dm3Module::Dm3Module() {
	for (unsigned int i=0; i<DM3_OPCODES; ++i) {
			Dm3Module::opcode_callbacks[i]=NULL;
	}
	dm3_instance = Dm3::Instance();
	Dm3Module::opcode_callbacks[OPCODE_REPORT] = &handle_report;
	Dm3Module::opcode_callbacks[OPCODE_SIREN] = &handle_siren;
	Dm3Module::opcode_callbacks[OPCODE_BATTERY] = &handle_batterylevel;
//	Dm3Module::opcode_callbacks[OPCODE_ULTRASONICS_REPORT] = &report_ultrasonic_sensors;
	Dm3Module::pid = mcc.register_opcode_callbacks(Dm3Module::opcode_callbacks, DM3_OPCODES);
}
