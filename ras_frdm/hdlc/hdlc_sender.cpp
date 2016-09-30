//
// Created by root on 9/6/16.
//
#include "frdm_communication.h"
#include "yahdlc.h"
#include <cstdio>
#include <ctime>
#include "yahdlc.h"
#include "hdlc_sender.h"

int hdlc_sender_send_sabm(int connectionId, unsigned char station_address) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_SARM;
    controlFrame.seq_no= 0;
    controlFrame.address = station_address;
    char frame[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int frameSize;
    //printf("Sender sending SABM...\n");
    if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize) == 0) {
        //printf("Sender yahdlc frame created OK\n");
        if (send_to_frdm(connectionId, frame, frameSize)) {
            //printf("SENDER send_to_frdm data OK\n");
            result = HDLC_OPERATION_OK;
        }
        else {
            //printf("SENDER send_to_frdm data ERROR!\n");
        }
    }
    else {
        //printf("Sender yahdlc frame created with ERROR!!\n");
    }

    return result;


}

int hdlc_sender_send_message(int connectionId, unsigned char station_address, int seq_number, char *data,
                             unsigned int dataLength) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    yahdlc_control_t controlFrame;
    controlFrame.frame = YAHDLC_FRAME_DATA;
    controlFrame.seq_no= (unsigned char) seq_number;
    controlFrame.address = station_address;
    char frame[_HDLC_MAX_MESSAGE_LENGTH];
    frame[0] = '\0';
    unsigned int frameSize;
    if(yahdlc_frame_data(&controlFrame, data, dataLength, frame, &frameSize) == 0) {
        if (send_to_frdm(connectionId, frame, frameSize)) {
            result = HDLC_OPERATION_OK;
        }
    }

    return result;
}

int hdlc_sender_read_ack(int connectionId, unsigned char station_address, int *max_time, int seq_number) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    char data_read[_HDLC_MAX_MESSAGE_LENGTH];
    //ToDo: consider timer
    bool timeout = false;
    yahdlc_control_t controlFrame;
    int data_read_length;
    char data_read_decoded[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int data_read_decoded_length;
    time_t start_time = 0;
    time_t end_time = 0;
    double time_diff = 0;

    /* Get the initial time */
    start_time = time(NULL);
    do {
        data_read_length = get_from_fdrm(connectionId, data_read, _HDLC_MAX_MESSAGE_LENGTH);
        if(data_read_length > 0) {
            if(yahdlc_get_data(&controlFrame, data_read, (unsigned int)data_read_length, data_read_decoded, &data_read_decoded_length)) {
                if(controlFrame.address != station_address
                   && controlFrame.frame == YAHDLC_FRAME_ACK
                   && controlFrame.seq_no == seq_number) {
                    result = HDLC_OPERATION_OK;
                }
            }
            //ToDo: remove this code when te timers are implemented
//            if(result != HDLC_OPERATION_OK)
//                timeout = true;
        }
        /* Get the end time after each iteration */
        end_time = time(NULL);
        /* Get the difference between times */
        time_diff = difftime(end_time, start_time);
        if(time_diff > *max_time) {
            /* timed out before getting an event */
            timeout = true;
            *max_time = 0;
        } else {
            *max_time = (int) (*max_time - time_diff);
        }
    } while (result != HDLC_OPERATION_OK && !timeout);

    return result;
}

int hdlc_sender_read_ua(int connectionId, unsigned char station_address, int *max_time) {
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
                if(controlFrame.address != station_address
                    && controlFrame.frame == YAHDLC_FRAME_UA) {
                    result = HDLC_OPERATION_OK;
                }
            }
            //ToDo: remove this code when te timers are implemented
//            if(result != HDLC_OPERATION_OK)
//                timeout = true;
        }

    } while (result != HDLC_OPERATION_OK && !timeout);

    return result;
}







