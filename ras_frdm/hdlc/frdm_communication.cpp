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
#include "frdm_communication.h"

Serial _serial_connection(USBTX, USBRX);

bool _serial_connection_callback_attached = false;

typedef struct {
	int index;
	char data[256];

}structured_data_t;

void serial_connection_callback() {

}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection(){

    return 1;
}

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize) {
	int data_read_index = 0;

	while(_serial_connection.readable() && (unsigned int)data_read_index < maxSize) {
		dataRead[data_read_index] = _serial_connection.getc();
		data_read_index++;
	}

	if(data_read_index < maxSize)
	{
		dataRead[data_read_index] = '\0';
	}
    return data_read_index;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size){
    int data_sent_index = 0;
    if(_serial_connection.writeable() && data_sent_index < size){
    	_serial_connection.putc(data[data_sent_index]);
    	data_sent_index++;
    }

    return data_sent_index;
}

int close_frdm_connection(int file_descriptor){
//    return _serial_connection.close();
	//_serial_connection.close();
	return 1;
}
