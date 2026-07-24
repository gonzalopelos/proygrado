/*******************************************************
 HDLC
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>     // Error number definitions
#include <unistd.h>
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
    printf("\t\"100\"-> Write 0000 and Read 1111\n");
    printf("\t\"101\"-> Write 0000\n");
    printf("\t\"102\"-> Write 1111\n");
    printf("\n");

    int command;
    do{
        scanf("%d", &command);
        ProcessComands(command);
    } while(command != 0);
}

void ProcessComands(int command){
    int res;
    int i,n;
    char* str;
    int file_descriptor;

    switch(command){
        case 0: // Quit command
            return;
            break;
        case 1: //Toggle LEDs command
            file_descriptor = open_frdm_connection();
            if(send_to_frdm(file_descriptor, "1", 1) == 1){
                printf("Toogle leds successfully\n");
            }else{
                printf("Error sending command\n");
            }
            close_frdm_connection(file_descriptor);
            break;
        case 2:

            char data_exa[256];
            unsigned int frame_length;
            frame_data("test", data_exa, &frame_length);

            if (res < 0){
                printf("Unable to write()\n");
                printf("Error:\n");
            }
            else{
                printf("Data sent successfully\nres: %d - frame_length: %d\n", res, frame_length);
                usleep(500*10000);
                res = 0;
                unsigned char data_read[256];
                while (res == 0) {
                    res = frame_length;
                    if (res == 0) {
                        printf("waiting...\n");
                    }
                    else if (res < 0){
                        printf("Unable to read()\n");
                    }
                    else {
                        printf("Data read (%d bytes):\n", res);
                        char frame[frame_length];
                        char data[frame_length];
                        unsigned int data_length = 0;
                        get_data(data_exa, frame_length, data, &data_length);
                        printf("Data read decoded (%d bytes):\n %s", data_length, data);
                    }
                    usleep(500*1000);
                }
            }


            break;

        default: return;
    }

}

void stringToHex(unsigned char* hexString, const char* string, int stringLenght){
    memset(hexString,0x00,stringLenght);
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

int frame_data(const char * send_data, char* frame_data,  unsigned int* frame_length){
    int ret;
    unsigned int data_length, seq_no = 0, frame_length2 = 0;
    data_length = sizeof(send_data);


    yahdlc_control_t control;
    yahdlc_frame_t frame_type = YAHDLC_FRAME_DATA;

    *frame_length = 0;




    /*if (!PyArg_ParseTuple(args, "s#|II", &send_data, &data_length, &frame_type, &seq_no))
        return NULL;

    if (data_length > MAX_FRAME_PAYLOAD)
    {
        PyErr_SetString(PyExc_ValueError, "data too long");
        return NULL;
    }
    else if (frame_type != YAHDLC_FRAME_DATA && frame_type != YAHDLC_FRAME_ACK && frame_type != YAHDLC_FRAME_NACK)
    {
        PyErr_SetString(PyExc_ValueError, "bad frame type");
        return NULL;
    }
    else if (seq_no < 0 || seq_no > 7)
    {
        PyErr_SetString(PyExc_ValueError, "invalid sequence number");
        return NULL;
    }*/

    control.frame = frame_type;
    control.seq_no = seq_no;


    ret = yahdlc_frame_data(&control, send_data, data_length, frame_data, frame_length);
    /* If success */
    if (ret == 0)
    {
        //frame_data_hexa = unsigned char [frame_length];
        //return PyBytes_FromStringAndSize(frame_data, frame_length);
        // else
        // {
        // 	PyErr_SetString(PyExc_ValueError, "invalid parameter");
        // 	return NULL;
    }
    else {
        printf("Error yahdlc_frame_data, ret:%d\n", ret);
        printf("yahdlc_frame_data, frame_length:%d\n", frame_length);
        printf("yahdlc_frame_data, data:%s, data_length:%d\n", send_data, data_length);

    }
    return ret;
}

int get_data (char* frame, unsigned int frame_length, char* data, unsigned int* data_length){
    int ret;
    yahdlc_control_t control;
    *data_length = 0;
    ret = yahdlc_get_data(&control, frame, frame_length, data, data_length);
}


//======================================================
