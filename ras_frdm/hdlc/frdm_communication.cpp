#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "Serial.h"
#include "mbed.h"
#include "rtos.h"
#include "frdm_communication.h"

Serial _serial_connection(USBTX, USBRX);
DigitalOut frdm_comm_led_green(LED2);
DigitalOut frdm_comm_led_blue(LED3);
DigitalOut frdm_comm_led4(LED4);

bool _serial_connection_callback_attached = false;

/** HDLC start/end flag sequence */
#define _YAHDLC_FLAG_SEQUENCE 0x7E

volatile const int _HDLC_MAX_MESSAGE_LENGTH = 256;
const int _BUFFER_MEESSAGE_CAPACITY = 10;

typedef struct {
	int size;
	char data[256];

} structured_data_t;

typedef struct {
	int connection_id;
	Queue<structured_data_t, 16> messages_queue;
	structured_data_t messages[_BUFFER_MEESSAGE_CAPACITY];
	MemoryPool<structured_data_t, _BUFFER_MEESSAGE_CAPACITY> messages_pool;
	int message_count;
} read_messages_buffer_t;

read_messages_buffer_t _read_message_buffers[2];

int _connections_count = 0;

Mutex _connection_mutex;
Mutex _read_message_buffers_mutex;

int started_message = 0;

unsigned char test_data[200];
int test_index = 0;
Mutex _test_mutex;


read_messages_buffer_t get_read_messages_buffer(int connection_id){
//	if(connection_id > _connections_count){
		//ToDo Handle Errors
//		return;
//	}
//	else {
		return _read_message_buffers[connection_id];
//	}
}

/*structured_data_t copy_message(structured_data_t message) {
	structured_data_t result;
	result.size = message.size;
	if (message.size > 0) {
		for (int index = 0; index < message.size; index++) {
			result.data[index] = message.data[index];
		}
	}
	if (message.size < _HDLC_MAX_MESSAGE_LENGTH) {
		result.data[message.size] = '\0';
	}
}*/

void delete_message(int connection_id, structured_data_t * message) {
	_read_message_buffers_mutex.lock();

	read_messages_buffer_t buffer = get_read_messages_buffer(connection_id);
	buffer.messages_pool.free(message);

	_read_message_buffers_mutex.unlock();
}

void add_message_in_buffers(structured_data_t message) {
	int new_index, index;
	_read_message_buffers_mutex.lock();
/*	for (int buffer_index = 0; buffer_index < _connections_count; buffer_index++) {
		if (_read_message_buffers[buffer_index].message_count < _BUFFER_MEESSAGE_CAPACITY) {
			_read_message_buffers[buffer_index].messages[_read_message_buffers[buffer_index].message_count] = copy_message(message);
			if(_read_message_buffers[0].messages[0].size > 0) {
				wait(1);
				frdm_comm_led_green = !frdm_comm_led_green;
				wait(1);
				frdm_comm_led_green = !frdm_comm_led_green;
				wait(1);
			}
			_read_message_buffers[buffer_index].message_count++;

		}
	}*/
	structured_data_t * new_meesage;
	for (index = 0; index < _connections_count; index++) {
		new_meesage = _read_message_buffers[index].messages_pool.alloc();
		new_meesage->size = message.size;
		for (new_index = 0; new_index < message.size; new_index++) {
			new_meesage->data[new_index] = message.data[new_index];
		}
//		sprintf(new_meesage->data, message.data);
		_read_message_buffers[index].messages_queue.put(new_meesage);
		frdm_comm_led_green = !frdm_comm_led_green;
		wait(0.5);
		frdm_comm_led_green = !frdm_comm_led_green;
	}

	_read_message_buffers_mutex.unlock();
}

bool try_get_message_from_buffer(int connection_id,	structured_data_t* message) {
	bool result = false;
	read_messages_buffer_t buffer = get_read_messages_buffer(connection_id);
	if (buffer.message_count > 0) {
		/*		message = (structured_data_t*) malloc(sizeof(structured_data_t));
		for (int data_index = 0; data_index	< _read_message_buffers[connection_id].messages[0].size; data_index++) {
			message->data[data_index] =	_read_message_buffers[connection_id].messages[0].data[data_index];
		}
		message->size = _read_message_buffers[connection_id].messages[0].size;
		if(_read_message_buffers[connection_id].messages[0].size > 0) {
			frdm_comm_led_blue = !frdm_comm_led_blue;
			wait(0.5);
			frdm_comm_led_blue = !frdm_comm_led_blue;
		}
//		for (int index = 0;	index < _read_message_buffers[connection_id].message_count - 1;	++index) {
//			_read_message_buffers[connection_id].messages[index] = copy_message(_read_message_buffers[connection_id].messages[index + 1]);
//		}
 */
		osEvent e = buffer.messages_queue.get();
		if (e.status == osEventMessage) {
			message = (structured_data_t*)e.value.p;
			_read_message_buffers[connection_id].message_count--;
			result = true;
		}
	}

	return result;
}

void serial_connection_callback() {
	//char read_char;
	_test_mutex.lock();
	structured_data_t message;
	while (_serial_connection.readable()) {
		unsigned char read_char = _serial_connection.getc();

		test_data[test_index] = read_char;
		test_index++;
		if (started_message == 0 && read_char == _YAHDLC_FLAG_SEQUENCE) {
			started_message = 1;
			message.size = 1;
			message.data[0] = read_char;

		} else if (started_message == 1 && read_char != _YAHDLC_FLAG_SEQUENCE) {
			message.data[message.size] = read_char;
			message.size++;
		} else if (started_message == 1 && read_char == _YAHDLC_FLAG_SEQUENCE) {
			started_message = 0;
			message.data[message.size] = read_char;
			message.size++;
			add_message_in_buffers((message));
		}
	}
	_test_mutex.unlock();
}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection() {
	int connection_id = _connections_count;
	_connection_mutex.lock();

	if (_connections_count == 0) {
		_serial_connection.baud(9600);
		_serial_connection.format();
		_serial_connection.attach(serial_connection_callback);
	}
	_read_message_buffers[_connections_count].connection_id = connection_id;
	_read_message_buffers[_connections_count].message_count = 0;
	_connections_count++;

	_connection_mutex.unlock();

	return connection_id;
}

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize) {
	int data_read_size = 0;
	structured_data_t * message;
	_read_message_buffers_mutex.lock();

	if (try_get_message_from_buffer(file_descriptor, message)) {
		frdm_comm_led_green = !frdm_comm_led_green;
		wait(1);
		frdm_comm_led_green = !frdm_comm_led_green;
		if(message->size>0){
			frdm_comm_led_blue = !frdm_comm_led_blue;
			wait(1);
			frdm_comm_led_blue = !frdm_comm_led_blue;
		}
		for (int index = 0; index < message->size; index++) {
			dataRead[index] = message->data[index];
		}
		data_read_size = message->size;
		if (data_read_size < maxSize) {
			dataRead[data_read_size] = '\0';
		}
	} else {
		dataRead[0] = '\0';
	}

	_read_message_buffers_mutex.unlock();

	if(data_read_size>0){
		delete_message(file_descriptor, message);
		frdm_comm_led_blue = !frdm_comm_led_blue;
		wait(1);
		frdm_comm_led_blue = !frdm_comm_led_blue;
	}

	return data_read_size;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size) {
	int data_sent_index = 0;
//    while(_serial_connection.writeable() && (unsigned int)data_sent_index < size) {
//     _serial_connection.putc(data[data_sent_index]);
//     data_sent_index++;
//    }
	if (_serial_connection.writeable()) {
		data_sent_index = _serial_connection.puts(data);
	}

	return data_sent_index;
}
