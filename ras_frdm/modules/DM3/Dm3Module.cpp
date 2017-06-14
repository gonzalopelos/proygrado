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
using namespace utilities;

extern Mcc mcc;


Dm3 * dm3_instance;
Dm3Security* dm3_security_instance;

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

void Dm3Module::siren_task() {
    while (true) {
        if (siren_count==0 && dm3_security_info.status == ENABLED) {
    		siren_on = 0;
    		dm3_instance->horn(siren_on);
    	} else {
    		if(siren_count > 0 || dm3_security_info.status == WARNING || dm3_security_info.status == DISABLED){
    			siren_on = 1-siren_on;
    			dm3_instance->horn(siren_on);
    		}
    		if (siren_count > 0 && dm3_security_info.status == ENABLED){
    			siren_count--;
				mcc.encoder.startFrame();
				mcc.encoder.push(DM3_PID);
				mcc.encoder.push(OPCODE_SIREN);
				mcc.encoder.startList();
				mcc.encoder.push(siren_count);
				mcc.encoder.endList();
				mcc.encoder.endFrame();
    		}

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

static int report_dm3_security_state(unsigned int pid, unsigned int opcode){
	mcc.encoder.startFrame();
	mcc.encoder.push(DM3_PID);
	mcc.encoder.push(OPCODE_SECURITY);
	mcc.encoder.startList();
//	int len = snprintf(stringbuffer, STRING_BUFF_SIZE, "SECURITY STATE: %d", Dm3Security::get_instance()->get_state());
//	smcc.encoder.push(stringbuffer, len);
	mcc.encoder.endList();
	mcc.encoder.endFrame();

	return 1;
}

void Dm3Module::update_sd_status(dm3_security_device& sd) {
	bool enable = false;
	if(sd.status == DISABLED){
		if(dm3_security_info.status != DISABLED){
			//ToDo report disabel dm3
			dm3_instance->enable(0);
			dm3_security_info.status = DISABLED;
			report_dm3_security_status();
		}
	}else if(sd.status == WARNING){
		if(dm3_security_info.status == ENABLED){
			dm3_security_info.status = WARNING;
			//ToDo throw warning

			report_dm3_security_status();
		}
	}else if(sd.status == ENABLED){
		enable = true;
	}
	node * list_node;
	dm3_security_device * device;
	uint32_t index = 1;
	for (index = 1; index <= dm3_security_info.devices.length(); ++index) {
		list_node = dm3_security_info.devices.pop(index);
		device = (dm3_security_device*)list_node->data;
		enable &= device->status == ENABLED;
		if(device->type == sd.type && device->data.direction == sd.data.direction){
			dm3_security_info.devices.remove(index);
			dm3_security_info.devices.append(&sd);
			break;
		}
	}

	if(index > dm3_security_info.devices.length()){
		dm3_security_info.devices.append(&sd);
	}

	if(enable && dm3_security_info.status != ENABLED){
		//ToDo enable dm3;
		dm3_instance->enable(1);
		dm3_security_info.status = ENABLED;
		report_dm3_security_status();
	}
}

void Dm3Module::report_dm3_security_status() {
	mcc.encoder.startFrame();
	mcc.encoder.push(DM3_PID);
	mcc.encoder.push(OPCODE_SECURITY);
	mcc.encoder.startList();
	int len = snprintf(stringbuffer, STRING_BUFF_SIZE, "SECURITY STATE: %s",
			dm3_security_info.status == ENABLED ? "ENABLED"
					: dm3_security_info.status == WARNING ? "WARNING"
							: "DISABLED");
	mcc.encoder.push(stringbuffer, len);
	dm3_security_device * device;

	for(uint32_t dev_index = 1; dev_index <= dm3_security_info.devices.length(); ++dev_index){
		device = (dm3_security_device *)dm3_security_info.devices.pop(dev_index)->data;
		len = snprintf(stringbuffer, STRING_BUFF_SIZE, "[DEVICE,STATUS]: [%s, %s]", device->type == Dm3Security::ULTRASONIC ? "ULTRASONIC" : "BUMPER", device->status == ENABLED ? "ENABLED" : device->status == WARNING ? "WARNING" : "DISABLED");
		mcc.encoder.push(stringbuffer, len);
	}
	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

void Dm3Module::battery_report_task(void const *argument) {
	while(true){
		handle_batterylevel(DM3_PID, OPCODE_BATTERY);
		Thread::wait(BATT_SENSE_PERIOD);
	}

}

void Dm3Module::ultrasonic_distance_alert(Dm3Security::alert_data * data){
//	printf("ultrasonic_distance_alert: %d, %d, %d\n", data->level, data->direction, data->distance);
	dm3_security_device sd;
	sd.data.direction = data->direction;
	sd.data.distance = data->distance;
	sd.data.level = data->level;
	sd.status = data->level == Dm3Security::OK ? ENABLED : data->level == Dm3Security::WARNING ? WARNING : DISABLED;
	sd.type = Dm3Security::ULTRASONIC;
	update_sd_status(sd);
}

Dm3Module::Dm3Module() {
	for (unsigned int i=0; i<DM3_OPCODES; ++i) {
			Dm3Module::opcode_callbacks[i]=NULL;
	}
	dm3_security_info.status = ENABLED;
	dm3_instance = Dm3::Instance();
	dm3_security_instance = Dm3Security::get_instance();

	dm3_security_instance->attach(Dm3Security::ULTRASONIC, this, &Dm3Module::ultrasonic_distance_alert);

	Dm3Module::opcode_callbacks[OPCODE_REPORT] = &handle_report;
	Dm3Module::opcode_callbacks[OPCODE_SIREN] = &handle_siren;
	Dm3Module::opcode_callbacks[OPCODE_BATTERY] = &handle_batterylevel;
	Dm3Module::opcode_callbacks[OPCODE_SECURITY] = &report_dm3_security_state;
	Dm3Module::pid = mcc.register_opcode_callbacks(Dm3Module::opcode_callbacks, DM3_OPCODES);
}
