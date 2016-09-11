//
// Created by root on 9/6/16.
//
#include <frdm_communication.h>
#include <yahdlc.h>
#include "../includes/yahdlc.h"
#include "../includes/hdlc_sender.h"

int hdlc_sender_send_sabm(int connectionId) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_SARM;
    controlFrame.seq_no= 0;
    char frame[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int frameSize;
    if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize)) {
        if (send_to_frdm(connectionId, frame, frameSize)) {
            result = HDLC_OPERATION_OK;
        }
    }

    return result;


}

int hdlc_sender_send_message(int connectionId, int seq_number, char *data, unsigned int dataLength) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_DATA;
    controlFrame.seq_no= (unsigned char) seq_number;
    char frame[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int frameSize;
    if(yahdlc_frame_data(&controlFrame, data, dataLength, frame, &frameSize)) {
        if (send_to_frdm(connectionId, frame, frameSize)) {
            result = HDLC_OPERATION_OK;
        }
    }

    return result;
}

int hdlc_sender_read_ack(int connectionId, int *max_time, int seq_number) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    char data_read[_HDLC_MAX_MESSAGE_LENGTH];
    //ToDo: consider timer
    bool timeout = false;
    yahdlc_control_t controlFrame;
    int data_read_length;
    char data_read_decoded[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int data_read_decoded_length;
    do {
        data_read_length = get_from_fdrm(connectionId, data_read, _HDLC_MAX_MESSAGE_LENGTH);
        if(data_read_length > 0) {
            if(yahdlc_get_data(&controlFrame, data_read, (unsigned int)data_read_length, data_read_decoded, &data_read_decoded_length)) {
                if(controlFrame.frame == YAHDLC_FRAME_ACK && controlFrame.seq_no == seq_number) {
                    result = HDLC_OPERATION_OK;
                }
            }
            //ToDo: remove this code when te timers are implemented
            if(result != HDLC_OPERATION_OK)
                timeout = true;
        }

    } while (result != HDLC_OPERATION_OK && !timeout);

    return result;
}

int hdlc_sender_read_ua(int connectionId, int *max_time) {
    return 0;
}







