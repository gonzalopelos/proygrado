//
// Created by proygrado on 7/24/16.
//
#include <fcntl.h>
#include <stdio.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include "Serial.h"
#include "mbed.h"
#include "rtos.h"
#include "frdm_communication.h"

Serial _serial_connection(USBTX, USBRX);
DigitalOut frdm_comm_led2(LED2);
DigitalOut frdm_comm_led3(LED3);
DigitalOut frdm_comm_led4(LED4);

bool _serial_connection_callback_attached = false;

/** HDLC start/end flag sequence */
#define _YAHDLC_FLAG_SEQUENCE 0x7E

volatile const int _HDLC_MAX_MESSAGE_LENGTH = 256;
const int _BUFFER_MEESSAGE_CAPACITY = 10;

typedef struct {
	int size;
	char data[256];

}structured_data_t;

typedef struct {
	int connection_id;
	structured_data_t messages[_BUFFER_MEESSAGE_CAPACITY];
	int message_count;
}read_messages_buffer_t;

read_messages_buffer_t _read_message_buffers[2];

int _connections_count = 0;

Mutex _connection_mutex;
Mutex _read_message_buffers_mutex;



int started_message = 0;
structured_data_t * message;
unsigned char test_data[200];
int test_index = 0;
Mutex _test_mutex;




structured_data_t copy_message(structured_data_t message){
	structured_data_t result;
	result.size = message.size;
	if(message.size > 0) {
		for (int index = 0; index < message.size; ++index) {
			result.data[index] = message.data[index];
		}
	}
	if(message.size < _HDLC_MAX_MESSAGE_LENGTH) {
		result.data[message.size] = '\0';
	}
	frdm_comm_led2 = !frdm_comm_led2;
	wait(0.5);
	frdm_comm_led2 = !frdm_comm_led2;
}

void add_message_in_buffers(structured_data_t message) {
	_read_message_buffers_mutex.lock();

	frdm_comm_led2 = !frdm_comm_led2;
	wait(0.5);
	frdm_comm_led2 = !frdm_comm_led2;

	for (int buffer_index = 0; buffer_index < _connections_count; ++buffer_index) {
		if(_read_message_buffers[buffer_index].message_count < _BUFFER_MEESSAGE_CAPACITY) {
			_read_message_buffers[buffer_index].messages[_read_message_buffers[buffer_index].message_count] = copy_message(message);
			_read_message_buffers[buffer_index].message_count++;
		}
	}

	_read_message_buffers_mutex.unlock();
}

bool try_get_message_from_buffer(int connection_id, structured_data_t* message) {
	bool result = false;
	_read_message_buffers_mutex.lock();

	if(_read_message_buffers[connection_id].message_count > 0) {
		message = (structured_data_t*) malloc(sizeof(structured_data_t));
		for (int data_index = 0; data_index < _read_message_buffers[connection_id].messages[0].size; ++data_index) {
			message->data[data_index] = _read_message_buffers[connection_id].messages[0].data[data_index];
		}
		message->size = _read_message_buffers[connection_id].messages[0].size;
		for (int index = 0; index < _read_message_buffers[connection_id].message_count -1; ++index) {
			_read_message_buffers[connection_id].messages[index] = copy_message(_read_message_buffers[connection_id].messages[index+1]);
		}
		_read_message_buffers[connection_id].message_count --;
		result = true;
	}

	_read_message_buffers_mutex.unlock();

	return result;
}




void serial_connection_callback() {
	//char read_char;
	_test_mutex.lock();

	while(_serial_connection.readable()) {
		frdm_comm_led2 = !frdm_comm_led2;
//		wait(0.5);
		frdm_comm_led2 = !frdm_comm_led2;
		unsigned char read_char = _serial_connection.getc();

		test_data[test_index]=read_char;
		test_index++;
		if(started_message == 0 && read_char == _YAHDLC_FLAG_SEQUENCE) {
			message = (structured_data_t *)malloc(sizeof(structured_data_t));
			started_message = 1;
			message->size = 1;
			message->data[0] = read_char;


		} else if (started_message == 1 && read_char != _YAHDLC_FLAG_SEQUENCE){
			frdm_comm_led3 = !frdm_comm_led3;
			message->data[message->size] = read_char;
			message->size ++;
//			wait(0.2);
			frdm_comm_led3 = !frdm_comm_led3;
//			wait(0.2);
		} else if(started_message == 1 && read_char == _YAHDLC_FLAG_SEQUENCE) {
			frdm_comm_led4 = !frdm_comm_led4;
//			wait(0.2);
			frdm_comm_led4 = !frdm_comm_led4;
			started_message = 0;
			message->data[message->size] = read_char;
			message->size ++;
			add_message_in_buffers((*message));
			frdm_comm_led4 = !frdm_comm_led4;
//			wait(0.2);
			frdm_comm_led4 = !frdm_comm_led4;
		}
	}

	if(test_index>0 && message->size > 0 && started_message == 0){
		_serial_connection.puts((char*)test_data);
//		fflush(_serial_connection._file);
	}
	_test_mutex.unlock();
}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection(){
	_connection_mutex.lock();

	if(_connections_count == 0) {
		_serial_connection.baud(9600);
		_serial_connection.format();
		_serial_connection.attach(serial_connection_callback);
	}
	_read_message_buffers[_connections_count].connection_id = _connections_count +1;
	_read_message_buffers[_connections_count].message_count = 0;
	_connections_count ++;

	_connection_mutex.unlock();

    return _connections_count;
}

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize) {
	int data_read_size = 0;
	structured_data_t * message;
	_read_message_buffers_mutex.lock();

	if(try_get_message_from_buffer(file_descriptor, message)) {
		for (int index = 0; index < message->size; ++index) {
			dataRead[index] = message->data[index];
		}
		data_read_size = message->size;
		if(data_read_size < maxSize)
		{
			dataRead[data_read_size] = '\0';
		}
	} else {
		dataRead[0]='\0';
	}

	_read_message_buffers_mutex.unlock();

    return data_read_size;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size){
    int data_sent_index = 0;
//    while(_serial_connection.writeable() && (unsigned int)data_sent_index < size) {
//    	_serial_connection.putc(data[data_sent_index]);
//    	data_sent_index++;
//    }
    if(_serial_connection.writeable()) {
		data_sent_index = _serial_connection.puts(data);
    }

    return data_sent_index;
}

int close_frdm_connection(int file_descriptor){
//    return _serial_connection.close();
	//_serial_connection.close();
	return 1;
}
