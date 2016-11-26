/*
 * Logger.cpp
 *
 *  Created on: Nov 19, 2016
 *      Author: gonzalopelos
 */

#include <Logging/Logger.h>

#include "mbed.h"


#define MBED_DEV_IP       		"192.168.1.52"
#define MBED_DEV_MASK     		"255.255.255.0"
#define MBED_DEV_GW       		"0.0.0.0"
#define ECHO_SERVER_PORT   		5000
#define ALIVE_MESSAGE			"ALIVE\n"

Thread _socket_server_thread;
Mutex _buffer_message_mutex;

const int _MESSAGE_BUFFER_CAPACITY = 128;

EthernetInterface _eth;
TCPSocketServer _socket_server;
TCPSocketConnection _socket_client;

Logger* Logger::_logger_instance;

typedef struct {
	int size;
	char data[128];
} logger_message_t;


Queue<logger_message_t, _MESSAGE_BUFFER_CAPACITY> _messages;


void socket_server_thread_func();
logger_message_t*  get_message_to_send();
void remove_sent_message();

Logger::Logger() {
	init();
}

int Logger::write_trace(char* data) {
	int result = 0;
	logger_message_t* message_data = (logger_message_t*)malloc(sizeof(logger_message_t));
	bzero(message_data->data, sizeof(message_data->data));
	result = sprintf(message_data->data, data);
	message_data->size = result;

	_buffer_message_mutex.lock();
	_messages.put(message_data);

	_buffer_message_mutex.unlock();
	return result;
}

void Logger::init() {

	_eth.init(MBED_DEV_IP, MBED_DEV_MASK, MBED_DEV_GW); //Assign a device ip, mask and gateway
	_eth.connect(100);
	_socket_server.bind(ECHO_SERVER_PORT);
	_socket_server.listen();
	_socket_server_thread.start(&socket_server_thread_func);
}

Logger::~Logger() {
	free(_logger_instance);
}

Logger * Logger::get_instance() {
	if(Logger::_logger_instance == NULL) {
		Logger::_logger_instance = new Logger();
	}
	return _logger_instance;
}


void socket_server_thread_func() {

	logger_message_t* message;

	while (true) {
		_socket_server.accept(_socket_client);
		_socket_client.set_blocking(false, 1500); // Timeout after (1.5)s
		while(_socket_client.is_connected()){
			message = get_message_to_send();
			if( message != NULL){
				if(message->size > 0){
					_socket_client.send_all(message->data, message->size);
					free(message);
				} else {
					_socket_client.send_all("message size 0\n", 14);
				}
			}else {
				_socket_client.send_all(ALIVE_MESSAGE, 5);
			}
			wait(1);
		}
		_socket_client.close();
	}
}

logger_message_t* get_message_to_send() {
	logger_message_t* message = NULL;
	_buffer_message_mutex.lock();
	osEvent evt = _messages.get(500);
	if (evt.status == osEventMessage) {
		message = (logger_message_t*)evt.value.p;
	}
	_buffer_message_mutex.unlock();

	return message;
}

