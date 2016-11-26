/*
 * Logger.h
 *
 *  Created on: Nov 19, 2016
 *      Author: gonzalopelos
 */

#ifndef MODULES_LOGGING_LOGGER_H_
#define MODULES_LOGGING_LOGGER_H_

#include "rtos.h"
#include "EthernetInterface.h"

class Logger {

public:
	static Logger* get_instance();
	int write_trace(char* data);
	virtual ~Logger();
private:
	Logger();
	void init();
	static Logger* _logger_instance;
//	void socket_server_thread_func();
};
#endif /* MODULES_LOGGING_LOGGER_H_ */

