#include "Dm3Module.h"

#include <mbed.h>
#include "EmBencode.h"
#include <stdlib.h>
#include <string>

#include "Dm3.h"
Dm3 *dm3 = Dm3::Instance();

extern MCC mcc;

#define BATT_SENSE_PERIOD 1000*4
#define STRING_BUFF_SIZE 40
char stringbuff[STRING_BUFF_SIZE];

int siren_count = 0;
int siren_on = 0;

static int handle_siren(unsigned int  pid, unsigned int  opcode) {
	siren_count = mcc.incomming_params_n[0] * 2; // la multiplicacion es porque el siren_task espera medio segundo 
	return 1;
}

void Dm3Module::siren_task(void const *argument) {
    while (true) {
        if (siren_count==0) {
    		siren_on = 0;
    		dm3->horn(siren_on);
    	} else {
    		if (siren_count>0) siren_count--;
    		siren_on = 1-siren_on;
    		dm3->horn(siren_on);
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
	int len = sprintf(stringbuff, /*STRING_BUFF_SIZE,*/ "%.2f", 100.0*dm3->get_batt());	// 2016.11.04 AM - Cambio llamada a snprintf por llamada a sprintf
	mcc.encoder.push(stringbuff, len);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
	return 1;
}

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
	Dm3Module::opcode_callbacks[OPCODE_REPORT] = &handle_report;
	Dm3Module::opcode_callbacks[OPCODE_SIREN] = &handle_siren;
	Dm3Module::opcode_callbacks[OPCODE_BATTERY] = &handle_batterylevel;
	Dm3Module::pid = mcc.register_opcode_callbacks(Dm3Module::opcode_callbacks, DM3_OPCODES);
}

