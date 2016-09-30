//
// Created by root on 9/6/16.
//

#ifndef SBC_HDLC_SENDER_H
#define SBC_HDLC_SENDER_H

int hdlc_sender_send_sabm(int connectionId, unsigned char station_address);

int hdlc_sender_read_ua(int connectionId, unsigned char station_address, int *max_time);

int hdlc_sender_read_ack(int connectionId, unsigned char station_address, int *max_time, int seq_number);

int hdlc_sender_send_message(int connectionId, unsigned char station_address, int seq_number, char *data,
                             unsigned int dataLength);

#endif //SBC_HDLC_SENDER_H
