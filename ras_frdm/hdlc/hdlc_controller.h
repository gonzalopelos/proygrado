//
// Created by root on 8/28/16.
//

#ifndef SBC_HDLC_CONTROLLER_H
#define SBC_HDLC_CONTROLLER_H

int hdlc_init(unsigned char stationAddress);

int hdlc_send_data(char * data, unsigned int data_length);

int hdlc_read_data(char *data_received, unsigned int *data_received_length);

#endif //SBC_HDLC_CONTROLLER_H
