/*
 * Dm3Module.h
 *
 *  Created on: May 2, 2017
 *      Author: gonzalopelos
 */

#ifndef MODULES_DM3_DM3MODULE_H_
#define MODULES_DM3_DM3MODULE_H_

#include "../Mcc/Mcc.h"
#include "Watchdog.h"
#include "Dm3Security.h"
#include "../../utilities/linkedlist.h"

using namespace utilities;
namespace modules {

//#define OPCODE_REPORT 0
#define OPCODE_SIREN 1
#define OPCODE_BATTERY 2
#define OPCODE_SECURITY 3
#define OPCODE_RESET 4
#define DM3_OPCODES 4

class Dm3Module {
private:
	typedef enum e_dm3_security_status{
		ENABLED = 0, WARNING = 1, DISABLED = 2
	} dm3_security_status;

	typedef struct s_dm3_security_device{
		Dm3Security::security_device_type type;
		dm3_security_status status;
		Dm3Security::alert_data data;
	}dm3_security_device;

	typedef struct s_dm3_security_information{
		dm3_security_status status;
		linked_list<node> devices;
	}dm3_security_information;

	dm3_security_information dm3_security_info;

	void update_sd_status(dm3_security_device* sd);

	//void report_dm3_security_status();

	void update_sd_info(dm3_security_device* sd_dest, dm3_security_device const& sd_source);
	int pid;
	OpcodeCallback opcode_callbacks[DM3_OPCODES];
	void report_siren();

	void init_devices_status();

public:
	Dm3Module();
	virtual ~Dm3Module();
	void siren_task();
	static void battery_report_task(void const *argument);
	void report_last_reset_source();
	void bumper_state_alert(Dm3Security::alert_data * data);
	void ultrasonic_distance_alert(Dm3Security::alert_data * data);
	void tcp_connection_alert(Dm3Security::alert_data * data);
	void speed_checks_alert(Dm3Security::alert_data * data);
	void report_dm3_security_status();
};

} /* namespace modules */

#endif /* MODULES_DM3_DM3MODULE_H_ */
