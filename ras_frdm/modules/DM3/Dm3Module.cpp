/*
 * Dm3Module.cpp
 *
 *  Created on: May 2, 2017
 *      Author: gonzalopelos
 */

#include <Dm3Module.h>
#include "Dm3.h"
#include "../EmBencode/EmBencode.h"
#include "fsl_rcm.h"

#include "rtos.h"


using namespace modules;
using namespace utilities;

extern Watchdog wdt;
extern Mcc mcc;


Dm3 * dm3_instance;
Dm3Security* dm3_security_instance;

#define BATT_SENSE_PERIOD 1000*4
#define STRING_BUFF_SIZE 40


char stringbuffer[STRING_BUFF_SIZE];

int siren_count = 0;
int siren_on = 0;

Mutex _update_status_mutex;


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

static int handle_reset_source(unsigned int  pid, unsigned int  opcode){
	uint32_t resetSource = wdt.getLastResetStatus() & (kRCM_SourceWdog | kRCM_SourcePin | kRCM_SourcePor);

	mcc.encoder.startFrame();
	mcc.encoder.push(DM3_PID);
	mcc.encoder.push(OPCODE_RESET);
	mcc.encoder.startList();
	int len = snprintf(stringbuffer, STRING_BUFF_SIZE, "%lu", (unsigned long)resetSource);
	mcc.encoder.push(stringbuffer, len);
	switch(resetSource){
		case kRCM_SourceWdog:
			memcpy(stringbuffer, "WATCHDOG TIMEOUT RESET", 23);
			len = 23;
			break;
		case kRCM_SourcePin:
			memcpy(stringbuffer, "EXTERNAL PIN RESET", 18);
			len = 18;
			break;
		case kRCM_SourcePor:
			memcpy(stringbuffer, "POWER ON RESET", 14);
			len = 14;
			break;
		default:
			memcpy(stringbuffer, "OTHER SOURCE RESET", 18);
			len = 18;
			break;
	}
	mcc.encoder.push(stringbuffer, len);
	mcc.encoder.endList();
	mcc.encoder.endFrame();
	return 1;
}

void Dm3Module::update_sd_status(dm3_security_device* sd) {
	_update_status_mutex.lock();

	bool update_to_enable = false;
	bool update_to_warning = false;
	bool report_status = false;
	bool new_sd = true;

	if(sd->status == DISABLED){
		if(dm3_security_info.status != DISABLED){
			//ToDo report disabel dm3
			dm3_security_instance->disable_dm3();
			dm3_security_info.status = DISABLED;
			if(sd->type != Dm3Security::TCP_CONNECTION){
				report_status = true;
			}
		}
	}else if(sd->status == WARNING){
		update_to_warning = true;
		if(dm3_security_info.status == ENABLED){
			dm3_security_info.status = WARNING;
			report_status = true;
			update_to_warning = false;
		}
	}else if(sd->status == ENABLED){
		update_to_enable = true;
		update_to_warning = true;
	}
	node * list_node;
	dm3_security_device * device;
	uint32_t index = 1;

	for (index = 1; index <= dm3_security_info.devices.length(); index++) {
		list_node = dm3_security_info.devices.pop(index);
		device = (dm3_security_device*)list_node->data;
		if((int)device->type == (int)sd->type && (int)device->data.direction == (int)sd->data.direction){
			update_sd_info(device, *sd);
			new_sd = false;
		}
		update_to_enable &= device->status == ENABLED;
		update_to_warning &= device->status != DISABLED;
	}

	if(update_to_enable){
		if(sd->type == Dm3Security::MOTORS_STATUS && dm3_security_info.status != ENABLED){
			dm3_security_info.status = ENABLED;
			dm3_security_instance->enable_dm3();
			report_status = true;
		}
		else if(dm3_security_info.status == WARNING){
			dm3_security_info.status = ENABLED;
			report_status = true;
		}
	} else if(update_to_warning && dm3_security_info.status == ENABLED) {
		dm3_security_info.status = WARNING;
		report_status = true;
	}

	if(new_sd){
		//printf("append sd: type %d, direction %d, status %d\n", sd->type, sd->data.direction, sd->status);
		dm3_security_info.devices.append(sd);
	}
	else{
		delete sd;
	}

	if(report_status){
	    report_dm3_security_status();
	}
	_update_status_mutex.unlock();
}

void Dm3Module::report_dm3_security_status() {
	mcc.encoder.startFrame();
	mcc.encoder.push(ADMIN_PID);
	mcc.encoder.push(OPCODE_SECURITY);
	mcc.encoder.startList();

	mcc.encoder.push(dm3_security_info.status);	// Report general status

	// Add each device status
	int len = 0;
	dm3_security_device * device;
	for(uint32_t dev_index = 1; dev_index <= dm3_security_info.devices.length(); ++dev_index){
		device = (dm3_security_device *)dm3_security_info.devices.pop(dev_index)->data;
		len = snprintf(stringbuffer, STRING_BUFF_SIZE, "%u,%u", device->type, device->status);
		mcc.encoder.push(stringbuffer, len);
	}
	// Add reset source.
	uint32_t resetSource = wdt.getLastResetStatus() & (kRCM_SourceWdog | kRCM_SourcePin | kRCM_SourcePor);
	len = snprintf(stringbuffer, STRING_BUFF_SIZE, "4,%lu", resetSource);
	mcc.encoder.push(stringbuffer, len);

	mcc.encoder.endList();
	mcc.encoder.endFrame();
}

void Dm3Module::battery_report_task(void const *argument) {
	while(true){
		handle_batterylevel(DM3_PID, OPCODE_BATTERY);
		Thread::wait(BATT_SENSE_PERIOD);
	}

}

void Dm3Module::report_last_reset_source(){
	handle_reset_source(DM3_PID, OPCODE_RESET);
}

void Dm3Module::bumper_state_alert(Dm3Security::alert_data * data){
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : data->level == Dm3Security::WARNING ? WARNING : DISABLED;
	sd->type = Dm3Security::BUMPER;

	update_sd_status(sd);
}

void Dm3Module::ultrasonic_distance_alert(Dm3Security::alert_data * data){
//	//printf("ultrasonic_distance_alert: %d, %d, %d\n", data->level, data->direction, data->distance);
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.distance = data->distance;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : data->level == Dm3Security::WARNING ? WARNING : DISABLED;
	sd->type = Dm3Security::ULTRASONIC;

	update_sd_status(sd);
}

void Dm3Module::update_sd_info(dm3_security_device* sd_dest, const dm3_security_device& sd_source) {
	sd_dest->data.direction = sd_source.data.direction;
	sd_dest->data.distance = sd_source.data.distance;
	sd_dest->data.level = sd_source.data.level;
	sd_dest->status = sd_source.status;
	sd_dest->type = sd_source.type;
}

void Dm3Module::tcp_connection_alert(Dm3Security::alert_data* data) {
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.distance = data->distance;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : data->level == Dm3Security::WARNING ? WARNING : DISABLED;
	sd->type = Dm3Security::TCP_CONNECTION;
	update_sd_status(sd);
}

void Dm3Module::speed_checks_alert(Dm3Security::alert_data* data) {
//	printf("speed_checks_alert: %d, %d, %d\n", data->level, data->direction, data->distance);
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.distance = data->distance;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : data->level == Dm3Security::WARNING ? WARNING : DISABLED;
	sd->type = Dm3Security::SPEEDS_CHECK;
	update_sd_status(sd);
}

void Dm3Module::power_speed_inconsistency_alert(Dm3Security::alert_data * data)
{
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.distance = data->distance;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : data->level == Dm3Security::WARNING ? WARNING : DISABLED;
	sd->type = Dm3Security::POWER_CHECK;
	update_sd_status(sd);
}

void Dm3Module::init_devices_status(){
	dm3_security_info.status = ENABLED;

	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = Dm3Security::FRONT;
	sd->data.distance = 0;
	sd->data.level = Dm3Security::OK;
	sd->status = ENABLED;
	sd->type = Dm3Security::BUMPER;
	dm3_security_info.devices.append(sd);

	sd = new dm3_security_device();
	sd->data.direction = Dm3Security::FRONT;
	sd->data.distance = 0;
	sd->data.level = Dm3Security::OK;
	sd->status = ENABLED;
	sd->type = Dm3Security::ULTRASONIC;
	dm3_security_info.devices.append(sd);

	sd = new dm3_security_device();
	sd->data.direction = Dm3Security::FRONT;
	sd->data.distance = 0;
	sd->data.level = Dm3Security::OK;
	sd->status = ENABLED;
	sd->type = Dm3Security::TCP_CONNECTION;
	dm3_security_info.devices.append(sd);

	sd = new dm3_security_device();
	sd->data.direction = Dm3Security::FRONT;
	sd->data.distance = 0;
	sd->data.level = Dm3Security::OK;
	sd->status = ENABLED;
	sd->type = Dm3Security::SPEEDS_CHECK;
	dm3_security_info.devices.append(sd);

	sd = new dm3_security_device();
	sd->data.direction = Dm3Security::FRONT;
	sd->data.distance = 0;
	sd->data.level = Dm3Security::OK;
	sd->status = ENABLED;
	sd->type = Dm3Security::POWER_CHECK;
	dm3_security_info.devices.append(sd);
}

void Dm3Module::motors_status_alert(Dm3Security::alert_data * data){
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.distance = data->distance;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : DISABLED;
	sd->type = Dm3Security::MOTORS_STATUS;
	update_sd_status(sd);
}

void Dm3Module::i2c_motors_status_alert(Dm3Security::alert_data * data){
	dm3_security_device* sd = new dm3_security_device();
	sd->data.direction = data->direction;
	sd->data.distance = data->distance;
	sd->data.level = data->level;
	sd->status = data->level == Dm3Security::OK ? ENABLED : DISABLED;
	sd->type = Dm3Security::I2C_MOTORS_STATUS;
	update_sd_status(sd);
}

Dm3Module::Dm3Module() {
	for (unsigned int i=0; i<DM3_OPCODES; ++i) {
			Dm3Module::opcode_callbacks[i]=NULL;
	}

	init_devices_status();	// Inicializar informacion de seguridad.

	dm3_instance = Dm3::Instance();

	dm3_security_instance = Dm3Security::get_instance();

	dm3_security_instance->attach(Dm3Security::BUMPER, this, &Dm3Module::bumper_state_alert);
	dm3_security_instance->attach(Dm3Security::ULTRASONIC, this, &Dm3Module::ultrasonic_distance_alert);
	dm3_security_instance->attach(Dm3Security::TCP_CONNECTION, this, &Dm3Module::tcp_connection_alert);
	dm3_security_instance->attach(Dm3Security::SPEEDS_CHECK, this, &Dm3Module::speed_checks_alert);
	dm3_security_instance->attach(Dm3Security::POWER_CHECK, this, &Dm3Module::power_speed_inconsistency_alert);
	dm3_security_instance->attach(Dm3Security::MOTORS_STATUS, this, &Dm3Module::motors_status_alert);
	dm3_security_instance->attach(Dm3Security::MOTORS_STATUS, this, &Dm3Module::i2c_motors_status_alert);

	Dm3Module::opcode_callbacks[OPCODE_REPORT] = &handle_report;
	Dm3Module::opcode_callbacks[OPCODE_SIREN] = &handle_siren;
	Dm3Module::opcode_callbacks[OPCODE_BATTERY] = &handle_batterylevel;
//	Dm3Module::opcode_callbacks[OPCODE_SECURITY] = &report_dm3_security_status;
	Dm3Module::opcode_callbacks[OPCODE_RESET] = &handle_reset_source;
	Dm3Module::pid = mcc.register_opcode_callbacks(Dm3Module::opcode_callbacks, DM3_OPCODES);

	report_dm3_security_status();	// Reportar status inicial de los dispositivos.
}

