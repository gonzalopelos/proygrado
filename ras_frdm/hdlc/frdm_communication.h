//
// Created by proygrado on 7/24/16.
//

#ifndef SBC_FRDM_COMUNICATION_H
#define SBC_FRDM_COMUNICATION_H


/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */
int open_frdm_connection();

int get_from_fdrm(int file_descriptor, char *dataRead, unsigned int maxSize);

int send_to_frdm(int file_descriptor, char* data, unsigned int size);

int close_frdm_connection(int file_descriptor);

#endif //SBC_FRDM_COMUNICATION_H
