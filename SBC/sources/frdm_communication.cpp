//
// Created by proygrado on 7/24/16.
//
#include <fcntl.h>
#include <stdio.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <zconf.h>
#include "frdm_communication.h"
const char* frdm_devicePath = "/dev/ttyACM0";
/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection(){
    int res;
    /* Device_Name is a string such as "/dev/ttyUSB0" */
    if((res = open(frdm_devicePath, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
        char errorDesc[200];
        sprintf(errorDesc, "Failed to open port %s:%d:%s\n", frdm_devicePath, errno, strerror(errno));
        perror(errorDesc);
        res = -1;
    }
    else {
        fcntl(res, F_SETFL, 0);
    }
    return res;
}

int get_from_fdrm(int file_descriptor, char* dataRead, unsigned int maxSize){
    int res = -1;
    // Most USB serial port implementations use a max of 4096 for the buffer

    res = read(file_descriptor, dataRead, maxSize);

    /* 0 means no data - errno can indicate 2 or 11 and not be a problem, other values are bad */
    /* -1 means error, check errno */
    /* >0 means you got data at inPtr, which is the start of inBuf and the value indicates how much data you got */

    return res;
}

int send_to_frdm(int file_descriptor, char* data, unsigned int size){
    int res = write(file_descriptor, data, size);
    return res;
}

int close_frdm_connection(int file_descriptor){
    return close(file_descriptor);
}
