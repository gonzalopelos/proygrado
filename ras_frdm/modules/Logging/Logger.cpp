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

Thread _socket_server_thread;
//Mutex _buffer_message_mutex;

const int _MESSAGE_BUFFER_CAPACITY = 128;

EthernetInterface _eth;
TCPSocketServer _socket_server;
TCPSocketConnection _socket_client;

Logger* Logger::_logger_instance;

typedef struct {
	int size;
	char data[128];
} logger_message_t;


Mail<logger_message_t, _MESSAGE_BUFFER_CAPACITY> _messages;
int _allive_message_counter;

void socket_server_thread_func();
logger_message_t*  get_message_to_send();

int get_allive_message(char * message);

Logger::Logger() {
	init();
}

int Logger::write_trace(char* data) {
	int result = 0;
//	_buffer_message_mutex.lock();

	logger_message_t* message_data = _messages.alloc();
	if(message_data != NULL){
		bzero(message_data->data, sizeof(message_data->data));
		result = sprintf(message_data->data, data);
		message_data->size = result;
		_messages.put(message_data);

	}

//	_buffer_message_mutex.unlock();

	return result;
}

void Logger::init() {

	_eth.init(MBED_DEV_IP, MBED_DEV_MASK, MBED_DEV_GW); //Assign a device ip, mask and gateway
	_eth.connect(100);
	_socket_server.bind(ECHO_SERVER_PORT);
	_socket_server.listen();
	_socket_server_thread.start(&socket_server_thread_func);
	_allive_message_counter = 0;
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
	char allive_message[100];
	int allive_message_size;
	while (1) {
		_socket_server.accept(_socket_client);
		_socket_client.set_blocking(false, 1500); // Timeout after (1.5)s
		while(_socket_client.is_connected()){
			message = get_message_to_send();
			if( message != NULL){
				if(message->size > 0){
					_socket_client.send_all(message->data, message->size);
				} else {
					_socket_client.send_all("message size 0\n", 14);
				}
				_messages.free(message);
			}else {
				allive_message_size = get_allive_message(allive_message);
				_socket_client.send_all(allive_message, allive_message_size);
			}
			wait(1);
		}
		_socket_client.close();
	}
}

logger_message_t* get_message_to_send() {
	logger_message_t* message = NULL;
//	_buffer_message_mutex.lock();

	osEvent evt = _messages.get(500);
	if (evt.status == osEventMail) {
		message = (logger_message_t*)evt.value.p;
	}

//	_buffer_message_mutex.unlock();

	return message;
}

int get_allive_message(char * allive_message) {
	int res = 0;
	bzero(allive_message, sizeof(allive_message));
	res = sprintf(allive_message, "ALLIVE: %d\n", _allive_message_counter);
	_allive_message_counter = (_allive_message_counter + 1) % 10000;
	return res;
}
