//
// Created by root on 9/6/16.
//

#ifndef SBC_HDLC_SENDER_H
#define SBC_HDLC_SENDER_H

int hdlc_send_sabm(int connectionId);

int hdlc_read_ua(int connectionId, int* max_time);

int hdlc_read_ack(int connectionId, int * max_time, int seq_number);

int hdlc_send_message(int connectionId, char* data, int dataLength);

#endif //SBC_HDLC_SENDER_H
