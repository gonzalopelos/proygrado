/*
 * Admin.h
 *
 *  Created on: Feb 23, 2017
 *      Author: gonzalopelos
 */

#ifndef MODULES_ADMIN_ADMIN_H_
#define MODULES_ADMIN_ADMIN_H_

#include "Mcc.h"

namespace modules {

	//#define OPCODE_REPORT 0
	#define OPCODE_PING 1
	#define OPCODE_RESET 2
	#define ADMIN_OPCODES 3


	class Admin {
	private:
		int pid;
		OpcodeCallback opcode_callbacks[ADMIN_OPCODES];
		static Admin * admin_instance;
		Admin();
		virtual ~Admin();
	public:
		static void report_status(char * str, int len);
		static Admin* get_instance();
	};

} /* namespace modules */

#endif /* MODULES_ADMIN_ADMIN_H_ */
