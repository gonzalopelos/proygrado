/*
 * Admin.cpp
 *
 *  Created on: Feb 23, 2017
 *      Author: gonzalopelos
 */


//namespace modules {
	#include "Admin.h"
	#include "mbed.h"
	#include "../EmBencode/EmBencode.h"
	#include "modules/Mcc/Mcc.h"
	extern Mcc mcc;
//	using namespace modules::Admin;

	static int handle_report(unsigned int  pid, unsigned int  opcode) {
		//report_status(ok_str, sizeof(ok_str)-1);
		mcc.encoder.startFrame();
		mcc.encoder.push((long int)ADMIN_PID);
		mcc.encoder.push(OPCODE_REPORT);
		mcc.encoder.startList();
		//mcc.encoder.push(admin_str, sizeof(admin_str)-1);
		//mcc.encoder.push(MODULE_VERSION);
		mcc.encoder.push(ok_str, sizeof(ok_str)-1);
		mcc.encoder.endList();
		mcc.encoder.endFrame();
		return 1;
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
		wait(1); // espera a que se mande el mensaje

//		mbed_reset();
		return 1;
	}

	void Admin::report_status(char * str, int len) {
		mcc.encoder.startFrame();
		mcc.encoder.push(ADMIN_PID);
		mcc.encoder.push(OPCODE_REPORT);
		mcc.encoder.startList();
		//mcc.encoder.push(admin_str, sizeof(admin_str)-1);
		//mcc.encoder.push(MODULE_VERSION);
		mcc.encoder.push(str, len);
		mcc.encoder.endList();
		mcc.encoder.endFrame();
	}
	Admin::Admin() {
		for (int i=0; i<ADMIN_OPCODES; ++i) {
			Admin::opcode_callbacks[i]=NULL;
		}
		Admin::opcode_callbacks[OPCODE_REPORT] = &handle_report;
		Admin::opcode_callbacks[OPCODE_PING] = &handle_ping;
		Admin::opcode_callbacks[OPCODE_RESET] = &handle_reset;
		Admin::pid = mcc.register_opcode_callbacks(Admin::opcode_callbacks, ADMIN_OPCODES);
	}

	Admin::~Admin() {
		bzero(Admin::opcode_callbacks, ADMIN_OPCODES);
		Admin::pid = -1;
	}

//} /* namespace modules */
