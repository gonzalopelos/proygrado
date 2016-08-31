//
// Created by root on 8/28/16.
//


#include <zconf.h>
#include <yahdlc.h>
#include "../includes/hdlc_controller.h"
#include "../includes/yahdlc.h"
#include "../includes/frdm_communication.h"

int connectionID;
const unsigned int _TIME_OUT = 200;
const int _MAX_MESSAGE_LENGTH = 256;
/** HDLC station address */
#define YAHDLC_STATION_ADDR 0x01

int hdlc_init(unsigned char * secondaryStationAddress) {
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
    return 0;
}

//int hdlc_read_frame()