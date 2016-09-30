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
const char* frdm_devicePath = "/Users/gonzalopelos/Documents/ProyGrado/HDLC_Test/comunication.txt";

Serial _serail_connection(USBTX, USBRX);

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection(){

    return 1;
}

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize) {


    _serail_connection.gets(dataRead, maxSize);

    return 1;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size){
    int res = -1;

    res = _serail_connection.puts(data);

    return res;
}

int close_frdm_connection(int file_descriptor){
//    return _serail_connection.close();
	return 1;
}
