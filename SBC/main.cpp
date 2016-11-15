/*******************************************************
 HDLC
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>     // Error number definitions
#include <unistd.h>
#include <fstream>
#include <thread>

#include <hdlc_controller.h>
#include <fcntl.h>
#include <yahdlc.h>
#include "includes/yahdlc.h"
#include "frdm_communication.h"





#define MAX_FRAME_PAYLOAD		512
#define HEADERS_LENGTH			8

#define TOTAL_FRAME_LENGTH		MAX_FRAME_PAYLOAD + HEADERS_LENGTH

#define BAUDRATE B38400
/* change this definition for the correct port */
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

#define MAX_STR 255







//======================================================
//Definicion de operaciones auxiliares

void ReadCommands();
void ProcessComands(int command);
void stringToHex(unsigned char* hexString, const char* string, int stringLenght);
void hexToString(char* string, unsigned char* hexString, int stringLenght);

int frame_data(const char * send_data, char* frame_data, unsigned int* frame_data_length);

int get_data (char* frame, unsigned int frame_length, char* data, unsigned int* data_length);


void start_primary_station();
void start_secondary_station();
void test_hdlc_frdm();



int main(int argc, char* argv[])
{

    ReadCommands();

    return 0;
}

//======================================================
// Implementacion de operaciones auxiliares

void ReadCommands(){
    printf("\n\n******************************************************\n");
    printf("******************************************************\n");
    printf("Comandos:\n");
    printf("\t\"0\"-> Quit\n");
    printf("\t\"1\"-> Toggle LEDs\n");
    printf("\t\"2\"-> Test encode and decode data with yahdlc\n");
    printf("\t\"3\"-> Start primary station for HDLC communication\n");
    printf("\t\"4\"-> Start secondary station for HDLC communication\n");
    printf("\n");

    int command;
    do{
        scanf("%d", &command);
        ProcessComands(command);
    } while(command != 0);
}

void ProcessComands(int command){
    int file_descriptor;

    switch(command){
        case 0: // Quit command
            return;
        case 1: //Toggle LEDs command
            file_descriptor = open_frdm_connection();
            char data_read[256];
//            if(get_from_fdrm(file_descriptor, data_read, 255)) {
//                printf("Data read: %s\n", data_read);
//            } else {
//                printf("Nothing to read\n");
//            }

            if(send_to_frdm(file_descriptor, (char *) "message 1", 9) == 9){
                printf("Sent: message 1\n");
                if(send_to_frdm(file_descriptor, (char *) "message 2", 9) == 9) {
                    printf("Sent: message 2\n");
                    if(get_from_fdrm(file_descriptor, data_read, 255)) {
                        printf("Data read: %s\n", data_read);
                    } else {
                        printf("Nothing to read\n");
                    }
                }else {
                    printf("Error sending command\n");
                }
            }else{
                printf("Error sending command\n");
            }
            close_frdm_connection(file_descriptor);
            break;
        case 2:
            char data_exa[256];
            unsigned int frame_length;
            yahdlc_control_t control;
            control.address = 0x2;
            control.frame = YAHDLC_FRAME_DATA;
            control.seq_no = 1;
            data_exa[0] = '\0';
            yahdlc_frame_data(&control,"test", 4, data_exa, &frame_length);
            yahdlc_control_t control2;
            char data[256];
            data[0] = '\0';
            unsigned int data_length;
            yahdlc_get_data(&control2, data_exa, frame_length, data, &data_length);
            printf("Data: %4s\n", data);
            break;
        case 3:

            start_primary_station();
            break;
        case 4:
            start_secondary_station();
            break;
        case 5:
            test_hdlc_frdm();
            break;

        default:break;
    }
}

void stringToHex(unsigned char* hexString, const char* string, int stringLenght){
    memset(hexString, 0x00, (size_t) stringLenght);
    strcpy((char*)hexString, string);

    for (int x = 0; x < stringLenght; x++)
    {
        printf("cahr: %c, hex %02hhx \n", string[x], hexString[x]);
    }
}


void hexToString(char* string, unsigned char* hexString, int stringLenght){
    memset(string, 0, stringLenght);
    strcpy(string, (char*)hexString);

    for (int x = 0; x < stringLenght; x++)
    {
        printf("cahr: %c, hex %02hhx \n", string[x], hexString[x]);
    }
}

//int frame_data(const char * send_data, char* frame_data,  unsigned int* frame_length){
//    int ret;
//    unsigned int data_length, seq_no = 0;
//    data_length = sizeof(send_data);
//
//
//    yahdlc_control_t control;
//    yahdlc_frame_t frame_type = YAHDLC_FRAME_DATA;
//
//    *frame_length = 0;
//
//
//
//
//    /*if (!PyArg_ParseTuple(args, "s#|II", &send_data, &data_length, &frame_type, &seq_no))
//        return NULL;
//
//    if (data_length > MAX_FRAME_PAYLOAD)
//    {
//        PyErr_SetString(PyExc_ValueError, "data too long");
//        return NULL;
//    }
//    else if (frame_type != YAHDLC_FRAME_DATA && frame_type != YAHDLC_FRAME_ACK && frame_type != YAHDLC_FRAME_NACK)
//    {
//        PyErr_SetString(PyExc_ValueError, "bad frame type");
//        return NULL;
//    }
//    else if (seq_no < 0 || seq_no > 7)
//    {
//        PyErr_SetString(PyExc_ValueError, "invalid sequence number");
//        return NULL;
//    }*/
//
//    control.frame = frame_type;
//    control.seq_no = seq_no;
//
//
//    ret = yahdlc_frame_data(&control, send_data, data_length, frame_data, frame_length);
//    /* If success */
//    if (ret != 0)
//    {
//        printf("Error yahdlc_frame_data, ret:%d\n", ret);
//        printf("yahdlc_frame_data, frame_length:%d\n", (int) (*frame_length));
//        printf("yahdlc_frame_data, data:%s, data_length:%d\n", send_data, data_length);
//
//    }
//    return ret;
//}

//int get_data (char* frame, unsigned int frame_length, char* data, unsigned int* data_length){
//    int ret;
//    yahdlc_control_t control;
//    *data_length = 0;
//    ret = yahdlc_get_data(&control, frame, frame_length, data, data_length);
//    return ret;
//}

void hdlc_controller_init_primary_station() {
    unsigned char primary_station_address = PRIMARY_STATION_ADDR;
    hdlc_init(primary_station_address);
}

void hdlc_controller_init_secondary_station() {
    unsigned char secondary_station_address = 0x2;
    hdlc_init(secondary_station_address);
}

void start_primary_station() {
    FILE * f = fopen("/Users/gonzalopelos/Documents/ProyGrado/HDLC_Test/comunication.txt","w+");
    fclose(f);
    std::thread primary_station_thread(hdlc_controller_init_primary_station);
    int primary_station_command;
    char data_read[256];
    char data_send[256];
    unsigned int data_read_length;
    unsigned int data_send_length;
    int send_result;
    do{
        printf("Enter 1 to send data\nEnter 2 to read data\n");
        scanf("%d", &primary_station_command);
        if(primary_station_command == 1) {
            scanf("%4s", data_send);
            data_send_length = 4;
            send_result = hdlc_send_data(data_send, data_send_length);
            if(send_result) {
                printf("Data send: %s\nResult: %d\n", data_send, send_result);
            } else {
                printf("Error send data\n");
            }
        } else {
            data_read[0] = '\0';
            data_read_length = 0;
            hdlc_read_data(data_read, &data_read_length);
            printf("Data read: %s\nData read length: %d\n", data_read, data_read_length);
        }
    } while(primary_station_command != 0);

    primary_station_thread.join();
}

void start_secondary_station() {
    std::thread secondary_station_thread(hdlc_controller_init_secondary_station);

    int primary_station_command;
    char data_read[256];
    char data_send[256];
    unsigned int data_read_length;
    unsigned int data_send_length;
    int send_result;
    do{
        printf("Enter 1 to send data\nEnter 2 to read data\n");
        scanf("%d", &primary_station_command);
        if(primary_station_command == 1) {
            scanf("%4s", data_send);
            data_send_length = 4;
            send_result = hdlc_send_data(data_send, data_send_length);
            if(send_result) {
                printf("Data send: %s\nResult: %d\n", data_send, send_result);
            } else {
                printf("Error send data\n");
            }
        } else {
            data_read[0] = '\0';
            data_read_length = 0;
            hdlc_read_data(data_read, &data_read_length);
            printf("Data read: %s\nData read length: %d\n", data_read, data_read_length);
        }
    } while(primary_station_command != 0);

    secondary_station_thread.join();
}

void test_hdlc_frdm() {
    char data5[256];
    yahdlc_control_t control4;
    control4.address = 0x2;
    control4.frame = YAHDLC_FRAME_DATA;
    control4.seq_no = 1;
    data5[0] = '\0';
    char data_read2[256];
    data_read2[0] = '\0';
    yahdlc_control_t control5;
    control5.address = 0x2;
    control5.frame = YAHDLC_FRAME_DATA;
    control5.seq_no = 2;
    char decoded_data[256];
    unsigned int decoded_data_size;
    unsigned int frame_length;

    yahdlc_frame_data(&control4,"hola mundo", 10, data5, &frame_length);
    int connection_id = open_frdm_connection();
    if(connection_id) {
        if(send_to_frdm(connection_id, data5, frame_length)) {
            printf("Data sent successfully. Data size %d\n", frame_length);
            unsigned int frame_size = get_from_fdrm(connection_id, data_read2, 256);
            if(frame_size > 0) {
                yahdlc_get_data(&control5, data_read2, frame_size, decoded_data, &decoded_data_size);
                printf("Data read: %s\n", decoded_data);
            }
        }
        close_frdm_connection(connection_id);
    }

}


//======================================================
