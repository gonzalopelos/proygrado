//
// Created by root on 9/7/16.
//
#include <yahdlc.h>
#include <frdm_communication.h>
#include <cstring>
#include <cstdio>
#include "../includes/hdlc_receiver.h"


int hdlc_receiver_read_sabm(int connectionId, unsigned char station_address) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    char data_read[_HDLC_MAX_MESSAGE_LENGTH];
    //ToDo: consider timer
    bool time_out = false;
    yahdlc_control_t controlFrame;
    int data_read_length;
    char data_read_decoded[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int data_read_decoded_length;
    do {
        data_read_length = get_from_fdrm(connectionId, data_read, _HDLC_MAX_MESSAGE_LENGTH);
        if(data_read_length > 0) {
            if(yahdlc_get_data(&controlFrame, data_read, (unsigned int)data_read_length, data_read_decoded, &data_read_decoded_length)) {
                //ToDo: check from address
                if(controlFrame.address != station_address
                    && controlFrame.frame == YAHDLC_FRAME_SARM) {
                    result = HDLC_OPERATION_OK;
                }
            }
            //ToDo: remove this code when te timers are implemented
//            if(result != HDLC_OPERATION_OK)
//                time_out = true;
        }

    } while (result != HDLC_OPERATION_OK && !time_out);

    return result;
}

int hdlc_receiver_send_ua(int connectionId, unsigned char station_address) {
    //printf("Receiver sending UA frame\n");
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_UA;
    controlFrame.seq_no= 0;
    controlFrame.address = station_address;
    char frame[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int frameSize;
    if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize) == 0) {
        if (send_to_frdm(connectionId, frame, frameSize)) {
            result = HDLC_OPERATION_OK;
        }
        else {
            //printf("Receiver send UA frame with ERROR!\n");
        }
    }

    return result;
}

int hdlc_receiver_read_message(int connectionId, unsigned char station_address, int seqNumber, unsigned int max_length,
                               char *data, unsigned int *data_length) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    char data_read[_HDLC_MAX_MESSAGE_LENGTH];
    //ToDo: consider timer
    bool timeout = false;
    yahdlc_control_t controlFrame;
    int data_read_length;
    char data_read_decoded[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int data_read_decoded_length;
    do {
        data_read_length = get_from_fdrm(connectionId, data_read, max_length);
        if(data_read_length > 0) {
            if(yahdlc_get_data(&controlFrame, data_read, (unsigned int)data_read_length, data_read_decoded, &data_read_decoded_length)) {
                //ToDo: check from address
                if(controlFrame.address != station_address
                    && controlFrame.frame == YAHDLC_FRAME_DATA) {
                    if(controlFrame.seq_no == seqNumber) {
                        strcpy(data, data_read_decoded);
                        *data_length = data_read_decoded_length;
                        result = HDLC_OPERATION_OK;
                    } else {
                      //ToDo: handle message read with another sequence number.
                    }
                }
                else {
                    //ToDo: handle message read without data.
                }
            }
            else {
                //ToDo: can not decode message.
            }
            //ToDo: remove this code when te timers are implemented
//            if(result != HDLC_OPERATION_OK)
//                timeout = true;
        }

    } while (result != HDLC_OPERATION_OK && !timeout);

    return result;;
}

int hdlc_receiver_send_ack(int connectionId, unsigned char station_address, int seqNumber) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_ACK;
    controlFrame.seq_no= (unsigned char) seqNumber;
    controlFrame.address = station_address;
    char frame[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int frameSize;
    if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize) == 0) {
        if (send_to_frdm(connectionId, frame, frameSize)) {
            result = HDLC_OPERATION_OK;
        }
    }

    return result;
}







