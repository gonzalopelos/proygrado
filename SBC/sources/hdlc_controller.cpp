//
// Created by root on 8/28/16.
//


#include <yahdlc.h>
#include "../includes/hdlc_controller.h"
#include <thread>
#include <cstdlib>
#include <semaphore.h>
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
unsigned char _station_address;

/**================== SENDER VARIABLES ===============*/
hdlc_status_t _sender_status;
mutex _sender_status_nutex;
mutex _sender_buffer_mutex;
mutex _sender_wait_connection_mutex;
mutex _sender_wait_data_mutex;

char _sender_buffer[_HDLC_MAX_MESSAGE_LENGTH];
unsigned int _sender_buffer_index;
unsigned int _sender_seq_number;

/**=================================*/

/**================== RECEIVER VARIABLES ===============*/
hdlc_status_t _receiver_status;
mutex _receiver_status_mutex;
mutex _receiver_buffer_mutex;
mutex _receiver_wait_connection_mutex;

char _receiver_buffer[_HDLC_MAX_MESSAGE_LENGTH];
unsigned int _receiver_buffer_index;
unsigned int _receiver_seq_number;
/**=================================*/



/****************************************************/


/****************************************************
  CONTROLLER AUXILIAR FUNCTIONS
 ****************************************************/

void hdlc_sender();
void hdlc_receiver();
bool hdlc_primary_station() { return _station_address == PRIMARY_STATION_ADDR; }

/**=========== SENDER AUXILIAR FUNCTIONS ========================*/
hdlc_status_t hdlc_sender_get_status();
int hdlc_sender_initialize_connection(int sender_connection_id);
void hdlc_sender_update_status(hdlc_status_t status);
int hdlc_sender_buffer_add_data(const char * data, unsigned int data_length);
int hdlc_sender_buffer_get_data(char* data, unsigned int * data_length);
int hdlc_sender_buffer_remove_data(unsigned int index);
int hdlc_sender_buffer_clean();

/**=========== RECEIVER AUXILIAR FUNCTIONS ========================*/
hdlc_status_t hdlc_receiver_get_status();
int hdlc_receiver_initialize_connection(int receiver_connection_id);
void hdlc_receiver_update_status(hdlc_status_t status);
int hdlc_receiver_buffer_celan();
int hdlc_receiver_buffer_add_data(const char * data, unsigned int data_length);
unsigned int hdlc_receiver_buffuer_available_capacity();
/****************************************************/

/*******************************************************************************************
 ***************************** CONTROLLER API FUNCTIONS ********************************/

int hdlc_init(unsigned char station_address) {

    //buffers initialization
    hdlc_sender_buffer_clean();
    hdlc_receiver_buffer_celan();

    //sequence numbers initialization
    _sender_seq_number = _receiver_seq_number = 0;

    //status initialization
    hdlc_sender_update_status(HDLC_START_CONNECTION);
    hdlc_receiver_update_status(HDLC_START_CONNECTION);

    //station address initialization
    _station_address = station_address;

    //semaphores initialization for start connection
    if(hdlc_primary_station()) {
        _receiver_wait_connection_mutex.lock();
    }
    else {
        _sender_wait_connection_mutex.lock();
    }

    thread sender_thread(hdlc_sender);
    thread receiver_thread(hdlc_receiver);

    sender_thread.join();
    receiver_thread.join();

    return 0;
}

int hdlc_read_data(char *data_received, unsigned int *data_received_length) {
    _receiver_buffer_mutex.lock();

    strcpy(data_received, _receiver_buffer);
    *data_received_length = _receiver_buffer_index;
    _receiver_buffer_index = 0;
    _receiver_buffer[0] = '\0';

    _receiver_buffer_mutex.unlock();

    return 0;
}

int hdlc_send_data(char *data, unsigned int data_length) {
    int result = hdlc_sender_buffer_add_data(data, data_length);
    return result;
}


/*******************************************************************************************
 ***************************** CONTROLLER THREAD FUNCTIONS ********************************/

void hdlc_sender() {
    /*
     connectionID = open_frdm_connection();
    if(connectionID) {
        yahdlc_control_t controlFrame;
        controlFrame.frame = YAHDLC_FRAME_SARM;
        controlFrame.seq_no= 0;
        char frame[_HDLC_MAX_MESSAGE_LENGTH];
        unsigned int frameSize;
        if(yahdlc_frame_data(&controlFrame,"",0, frame, &frameSize)) {
            if(send_to_frdm(connectionID, frame, frameSize)) {
                char receivedFrame[_HDLC_MAX_MESSAGE_LENGTH];
                char receivedFrameData[_HDLC_MAX_MESSAGE_LENGTH];
                unsigned int receivedFrameDataLength;
                int readBytes = 0;
                while(!readBytes) {
                    readBytes = get_from_fdrm(connectionID, receivedFrame,_HDLC_MAX_MESSAGE_LENGTH);
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
    //ToDo: refactor to return Operation Result
    int sender_connectionId = open_frdm_connection();
    //printf("Sender connection id: %d\n", sender_connectionId);
    if(sender_connectionId <= 0) {
        return;
    }

    //the primary station still retains responsibility for error recovery, link setup, and link disconnection.
    if(hdlc_primary_station()) {
        if(hdlc_sender_initialize_connection(sender_connectionId) != HDLC_OPERATION_OK) {
            hdlc_sender_update_status(HDLC_DISCONNECTED);
            _receiver_wait_connection_mutex.unlock();
            return;
        } else {
            _receiver_wait_connection_mutex.unlock();
        }
    } else {
        //wait the connection initialized for receiver
//        while (hdlc_receiver_get_status() != HDLC_CONNECTED && hdlc_receiver_get_status() != HDLC_DISCONNECTED){
//        }
        //printf("sender waiting for the receiver\n");
        _sender_wait_connection_mutex.lock();
        hdlc_sender_update_status(hdlc_receiver_get_status());
    }
    char data[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int data_length;
    int max_time = _HDLC_TIME_OUT;
    while(hdlc_sender_get_status() == HDLC_CONNECTED) {
        hdlc_sender_buffer_get_data(data, &data_length);
        // there are data to send
        if(data_length) {
            do {
                //printf("Sender sending data: %s\n", data);
                if(hdlc_sender_send_message(sender_connectionId, _station_address, _sender_seq_number, data, data_length) == HDLC_OPERATION_OK) {
                    hdlc_sender_update_status(HDLC_READ_ACK);
                    //ToDo: add mutex to sequence numbers access
                    switch(hdlc_sender_read_ack(sender_connectionId, _station_address, &max_time, _sender_seq_number)) {
                        case HDLC_OPERATION_OK:
                            //ToDo: add mutex to sequence numbers access
                            //ToDo: check number mod 8 (3 bits).
                            _sender_seq_number++;
                            hdlc_sender_buffer_remove_data(data_length);
                            hdlc_sender_update_status(HDLC_CONNECTED);
                            break;
                        case HDLC_OPERATION_ERROR_NOT_FOUND:
                            if(max_time) {
                                //ToDo: check possible errors and DM
                                hdlc_sender_update_status(HDLC_DISCONNECTED);
                            }
                            break;
                    }

                }

            } while (hdlc_sender_get_status() == HDLC_READ_ACK);
        }
    }

}

void hdlc_receiver() {
    //ToDo: refactor to return Operation Result
    int receiver_connection_id = open_frdm_connection();
    if(receiver_connection_id <= 0) {
        return;
    }
    //printf("Receiver connection id: %d\n", receiver_connection_id);
    if(!hdlc_primary_station()) {
        hdlc_receiver_update_status(HDLC_START_CONNECTION);
        if(hdlc_receiver_initialize_connection(receiver_connection_id) != HDLC_OPERATION_OK) {
            hdlc_receiver_update_status(HDLC_DISCONNECTED);
            _sender_wait_connection_mutex.unlock();
            return;
        } else {
            _sender_wait_connection_mutex.unlock();
        }
    } else {
        //wait the connection initialized for sender
//        while (hdlc_sender_get_status() != HDLC_CONNECTED && hdlc_sender_get_status() != HDLC_DISCONNECTED);
        //printf("Receiver waiting for the sender\n");
        _receiver_wait_connection_mutex.lock();
        hdlc_receiver_update_status(hdlc_sender_get_status());
    }
    unsigned int receiver_buffer_capacity;
    char data_read[_HDLC_MAX_MESSAGE_LENGTH];
    unsigned int data_read_length;
    do {
        //printf("Rceiver in principal loop\n");
        receiver_buffer_capacity = hdlc_receiver_buffuer_available_capacity();
        if(receiver_buffer_capacity) {
            if(hdlc_receiver_read_message(receiver_connection_id, _station_address, _receiver_seq_number, receiver_buffer_capacity,
                                          data_read, &data_read_length) == HDLC_OPERATION_OK) {
                hdlc_receiver_buffer_add_data(data_read, data_read_length);
                switch(hdlc_receiver_send_ack(receiver_connection_id, _station_address, _receiver_seq_number)) {
                    case HDLC_OPERATION_OK:
                        //ToDo: add mutex to sequence numbers access
                        //ToDo: check number mod 8 (3 bits).
                        _receiver_seq_number++;
                        break;
                    case HDLC_OPERATION_ERROR_NOT_FOUND:
                        //ToDo: check errors and DM
                        hdlc_receiver_update_status(HDLC_DISCONNECTED);
                        break;
                }
            }
        }
    } while (hdlc_receiver_get_status() == HDLC_CONNECTED);


}

/*******************************************************************************************
 ***************************** SENDER AUXILIAR FUNCTIONS ********************************/

int hdlc_sender_initialize_connection(int sender_connection_id) {

    //printf("Sender initializing connection\n");

    int result = hdlc_sender_send_sabm(sender_connection_id, _station_address);
    if (result == HDLC_OPERATION_OK) {
        //printf("Sender send SABM OK\n");
        hdlc_sender_update_status(HDLC_READ_UA);
        int max_time = _HDLC_TIME_OUT;
        //printf("Sender reading UA\n");
        while (hdlc_sender_get_status() == HDLC_READ_UA && result == HDLC_OPERATION_OK) {
            switch (hdlc_sender_read_ua(sender_connection_id, _station_address, &max_time)) {
                case HDLC_OPERATION_OK:
                    hdlc_sender_update_status(HDLC_CONNECTED);
                    break;
                case HDLC_OPERATION_ERROR_NOT_FOUND:
                    //printf("Sender read UA NOT FOUND\n");
                    if (!max_time) {
                        result = hdlc_sender_send_sabm(sender_connection_id, _station_address);
                        if (result == HDLC_OPERATION_OK) {
                            max_time = _HDLC_TIME_OUT;
                        }
                        break;
                    }
                    else {
                        //printf("Sender reading UA TIME OUT\n");
                    }
            }
        }
    }

    return result;
}

void hdlc_sender_update_status(hdlc_status_t status) {
    _sender_status_nutex.lock();
    _sender_status = status;
    _sender_status_nutex.unlock();
    //printf("sender update status: %d\n", (int)status);
}

hdlc_status_t hdlc_sender_get_status() {
    hdlc_status_t result;
    _sender_status_nutex.lock();
    result = _sender_status;
    _sender_status_nutex.unlock();
    return result;
}

int hdlc_sender_buffer_add_data(const char *data, unsigned int data_length) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    _sender_buffer_mutex.lock();

    if(_sender_buffer_index + data_length <= _HDLC_MAX_MESSAGE_LENGTH) {
        strcat(_sender_buffer, data);
        _sender_buffer_index += data_length;
        if(_sender_buffer_index < _HDLC_MAX_MESSAGE_LENGTH) {
            _sender_buffer[_sender_buffer_index] = '\0';
        }
        result = HDLC_OPERATION_OK;
    }
    if(_sender_buffer_index > 0) {
        _sender_wait_data_mutex.unlock();
    }
    _sender_buffer_mutex.unlock();

    return result;
}

int hdlc_sender_buffer_get_data(char *data, unsigned int *data_length) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    _sender_wait_data_mutex.lock();
    _sender_buffer_mutex.lock();

    if(_sender_buffer_index) {
        data[0] = '\0';
        strcpy(data, _sender_buffer);
    }
    *data_length = _sender_buffer_index;
    result = HDLC_OPERATION_OK;

    if(_sender_buffer_index > 0) {
        _sender_wait_data_mutex.unlock();
    }

    _sender_buffer_mutex.unlock();

    return result;
}

int hdlc_sender_buffer_clean() {
    _sender_buffer_mutex.lock();

    _sender_buffer_index = 0;
    _sender_buffer[_sender_buffer_index] = '\0';

    _sender_buffer_mutex.unlock();
    return HDLC_OPERATION_OK;
}

int hdlc_sender_buffer_remove_data(unsigned int index) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;

    _sender_buffer_mutex.lock();

    //remove al data form sender buffer
    if(index == _sender_buffer_index) {
        _sender_buffer[0] = '\0';
        _sender_buffer_index = 0;
    }
    else {

        for (int i = index; i < _sender_buffer_index; ++i) {
            _sender_buffer[i - index] = _sender_buffer[i];
        }
        _sender_buffer_index -= index;
    }
    if(_sender_buffer_index == 0){
        _sender_wait_data_mutex.lock();
    }
    _sender_buffer_mutex.unlock();

    result = HDLC_OPERATION_OK;
    return result;
}


/*******************************************************************************************
 ***************************** RECEIVER AUXILIAR FUNCTIONS ********************************/

hdlc_status_t hdlc_receiver_get_status() {
    hdlc_status_t result;
    _receiver_status_mutex.lock();
    result = _receiver_status;
    _receiver_status_mutex.unlock();
    return result;
}

void hdlc_receiver_update_status(hdlc_status_t status) {
    _receiver_status_mutex.lock();
    _receiver_status = status;
    _receiver_status_mutex.unlock();
    //printf("Receiver update status: %d\n", (int)status);
}

int hdlc_receiver_buffer_celan() {
    _receiver_buffer_mutex.lock();
    _receiver_buffer_index = 0;
    _receiver_buffer[0] = '\0';
    _receiver_buffer_mutex.unlock();
    return HDLC_OPERATION_OK;
}

int hdlc_receiver_initialize_connection(int receiver_connection_id) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;
    do {
        switch (hdlc_receiver_read_sabm(receiver_connection_id, _station_address)) {
            case HDLC_OPERATION_OK:
                hdlc_receiver_send_ua(receiver_connection_id, _station_address);
                hdlc_receiver_update_status(HDLC_CONNECTED);
                result = HDLC_OPERATION_OK;
                break;
            case HDLC_OPERATION_ERROR_NOT_FOUND:
                //ToDo: check errors and DM
                hdlc_receiver_update_status(HDLC_DISCONNECTED);
                break;
        }

    } while (hdlc_receiver_get_status() == HDLC_START_CONNECTION);
    return result;
}

unsigned int hdlc_receiver_buffuer_available_capacity() {
    unsigned int result = 0;

    _receiver_buffer_mutex.lock();

    result = _HDLC_MAX_MESSAGE_LENGTH - _receiver_buffer_index;

    _receiver_buffer_mutex.unlock();

    return result;
}

int hdlc_receiver_buffer_add_data(const char *data, unsigned int data_length) {
    int result = HDLC_OPERATION_ERROR_NOT_FOUND;

    _receiver_buffer_mutex.lock();

    if(_receiver_buffer_index + data_length <= _HDLC_MAX_MESSAGE_LENGTH) {
        strcat(_receiver_buffer, data);
        _receiver_buffer_index += data_length;
        if(_receiver_buffer_index < _HDLC_MAX_MESSAGE_LENGTH) {
            _receiver_buffer[_receiver_buffer_index] = '\0';
        }
        result = HDLC_OPERATION_OK;
    }

    _receiver_buffer_mutex.unlock();

    return result;

}
