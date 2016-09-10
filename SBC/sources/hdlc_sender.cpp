//
// Created by root on 9/6/16.
//
#include <frdm_communication.h>
#include "../includes/yahdlc.h"
#include "../includes/hdlc_sender.h"

int hdlc_send_sabm(int connectionId) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_SARM;
    controlFrame.seq_no= 0;
    char frame[_MAX_MESSAGE_LENGTH];
    unsigned int frameSize;
    if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize)) {
        if (send_to_frdm(connectionId, frame, frameSize)) {
            result = HDLC_OPERATION_OK;
        }
    }

    return result;


}

int hdlc_send_message(int connectionId, char *data, int dataLength) {
    return 0;
}

int hdlc_read_ack(int connectionId, int *max_time, int seq_number) {
    return 0;
}

int hdlc_read_ua(int connectionId, int *max_time) {
    return 0;
}







