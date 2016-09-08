//
// Created by root on 9/7/16.
//

#ifndef SBC_HDLC_RECEIVER_H
#define SBC_HDLC_RECEIVER_H

int hdlc_read_sabm(int connectionId);

int hdlc_send_ua(int connectionId);

int hdlc_read_message(int connectionId, int seqNumber);

int hdlc_send_ack(int connectionId, int seqNumber);

#endif //SBC_HDLC_RECEIVER_H
