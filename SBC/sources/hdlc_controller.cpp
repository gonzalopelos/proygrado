//
// Created by root on 8/28/16.
//


#include <yahdlc.h>
#include "../includes/hdlc_controller.h"
#include <thread>
#include "../includes/frdm_communication.h"
#include "../includes/hdlc_receiver.h"
#include "../includes/hdlc_sender.h"

const unsigned int _TIME_OUT = 200;
typedef enum {
    HDLC_START_CONNECTION,
    HDLC_CONNECTED,
    HDLC_READ_ACK,
    HDLC_READ_UA,
    HDLC_DISCONNECTED
} hdlc_status_t;

hdlc_status_t sender_status;
hdlc_status_t receiver_status;

char sender_buffer[_MAX_MESSAGE_LENGTH];
int sender_buffer_index;

char receiver_buffer[_MAX_MESSAGE_LENGTH];
int receiver_buffer_index;


using namespace std;

/** HDLC station address */
#define YAHDLC_STATION_ADDR 0x01

int hdlc_init(unsigned char * station_address) {

    //buffers initialization
    receiver_buffer_index = sender_buffer_index = 0;
    //status initialization
    sender_status = receiver_status = HDLC_START_CONNECTION;

    thread sender_thread(hdlc_sender, station_address);
    thread receiver_thread(hdlc_receiver, station_address);

    sender_thread.join();
    receiver_thread.join();

    return 0;
}

int hdlc_read_data(char *data_received, unsigned int data_received_length) {
    return 0;
}

int hdlc_send_data(char *data, unsigned int data_length) {
    return 0;
}


int hdlc_sender(char* station_address) {
    /*
     connectionID = open_frdm_connection();
    if(connectionID) {
        yahdlc_control_t controlFrame;
        controlFrame.frame = YAHDLC_FRAME_SARM;
        controlFrame.seq_no= 0;
        char frame[_MAX_MESSAGE_LENGTH];
        unsigned int frameSize;
        if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize)) {
            if(send_to_frdm(connectionID, frame, frameSize)) {
                char receivedFrame[_MAX_MESSAGE_LENGTH];
                char receivedFrameData[_MAX_MESSAGE_LENGTH];
                unsigned int receivedFrameDataLength;
                int readBytes = 0;
                while(!readBytes) {
                    readBytes = get_from_fdrm(connectionID, receivedFrame,_MAX_MESSAGE_LENGTH);
                }
                yahdlc_control_t receivedControlFrame;
                yahdlc_frame_data(&receivedControlFrame,receivedFrame, readBytes, receivedFrameData, &receivedFrameDataLength);
                if(receivedControlFrame.frame == YAHDLC_FRAME_UA) {
                    return 1;
                }
            }
        }
    }
     */
    int sender_connectionID = open_frdm_connection();


    return 0;
}

int hdlc_receiver(char* station_address) {
    int receiver_connectionID;
    return 0;

}