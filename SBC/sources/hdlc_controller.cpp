//
// Created by root on 8/28/16.
//


#include <yahdlc.h>
#include "../includes/hdlc_controller.h"
#include <thread>
#include "../includes/frdm_communication.h"
#include "../includes/hdlc_receiver.h"
#include "../includes/hdlc_sender.h"

using namespace std;

/****************************************************
 * Global variables and structures
 ****************************************************/
volatile const unsigned int _HDLC_TIME_OUT = 200;
typedef enum {
    HDLC_START_CONNECTION,
    HDLC_CONNECTED,
    HDLC_READ_ACK,
    HDLC_READ_UA,
    HDLC_DISCONNECTED
} hdlc_status_t;

hdlc_status_t _sender_status;
hdlc_status_t _receiver_status;

mutex _sender_status_nutex;
mutex _receiver_status_mutex;

char _sender_buffer[_MAX_MESSAGE_LENGTH];
int _sender_buffer_index;

char _receiver_buffer[_MAX_MESSAGE_LENGTH];
int _receiver_buffer_index;

unsigned char* _station_address;

/****************************************************/


/****************************************************
 * Auxiliar Functions
 ****************************************************/

void hdlc_sender();
void hdlc_receiver();
bool hdlc_primary_station() { return (*_station_address) == PRIMARY_STATION_ADDR; }
int hdlc_initialyze_connection(int sender_connection_id);
hdlc_status_t hdlc_get_sender_status();
void hdlc_update_sender_status(hdlc_status_t status);
hdlc_status_t hdlc_get_receiver_status();
void hdlc_update_receiver_status(hdlc_status_t status);

/****************************************************/

int hdlc_init(unsigned char * station_address) {

    //buffers initialization
    _receiver_buffer_index = _sender_buffer_index = 0;
    //status initialization
    hdlc_update_sender_status(HDLC_START_CONNECTION);
    hdlc_update_receiver_status(HDLC_START_CONNECTION);

    //station address initialization
    _station_address = station_address;

    thread sender_thread(hdlc_sender);
    thread receiver_thread(hdlc_receiver);

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

void hdlc_sender() {
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
    int sender_connectionId = open_frdm_connection();
    if(sender_connectionId) {
        return;
    }

    //the primary station still retains responsibility for error recovery, link setup, and link disconnection.
    if(hdlc_primary_station()) {
        if(hdlc_initialyze_connection(sender_connectionId) != HDLC_OPERATION_OK) {
            hdlc_update_sender_status(HDLC_DISCONNECTED);
            return;
        }
    }
    while(hdlc_get_sender_status() == HDLC_CONNECTED) {
        //ToDo: mutuoexcluir el acceso a los buufers e indices.
        if(_sender_buffer_index) {
            
        }
    }

}

void hdlc_receiver() {
    int receiver_connectionID = open_frdm_connection();
    if(receiver_connectionID) {

    }
}

int hdlc_initialyze_connection(int sender_connection_id) {
    int result = hdlc_send_sabm(sender_connection_id);
    if (result == HDLC_OPERATION_OK) {
        hdlc_update_sender_status(HDLC_READ_UA);
        int max_time = _HDLC_TIME_OUT;
        while (hdlc_get_sender_status() == HDLC_READ_UA && result == HDLC_OPERATION_OK) {
            switch (hdlc_read_ua(sender_connection_id, &max_time)) {
                case HDLC_OPERATION_OK:
                    hdlc_update_sender_status(HDLC_CONNECTED);
                    break;
                case HDLC_OPERATION_ERROR_NOT_FOUND:
                    if (!max_time) {
                        result = hdlc_send_sabm(sender_connection_id);
                        if (result == HDLC_OPERATION_OK) {
                            max_time = _HDLC_TIME_OUT;
                        }
                        break;
                    }
            }
        }
    }
    return result;
}

void hdlc_update_receiver_status(hdlc_status_t status) {
    _receiver_status_mutex.lock();
    _sender_status = status;
    _receiver_status_mutex.unlock();
}

hdlc_status_t hdlc_get_receiver_status() {
    hdlc_status_t result;
    _receiver_status_mutex.lock();
    result = _receiver_status;
    _receiver_status_mutex.unlock();
    return result;
}

void hdlc_update_sender_status(hdlc_status_t status) {
    _sender_status_nutex.lock();
    _sender_status = status;
    _sender_status_nutex.unlock();
}

hdlc_status_t hdlc_get_sender_status() {
    hdlc_status_t result;
    _sender_status_nutex.lock();
    result = _sender_status;
    _sender_status_nutex.unlock();
    return result;
}
