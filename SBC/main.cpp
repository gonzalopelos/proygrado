//#define MAIN_AM
/*******************************************************
 HDLC
********************************************************/
#ifdef MAIN_AM
#include <stdio.h>
#include "frdm_communication.h"


//======================================================
//Definicion de operaciones auxiliares

void ReadCommands();
void ProcessComands(int command);

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
    printf("\t\"2\"-> Test encode and decode data with BEncode\n");
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
            if(send_to_frdm(file_descriptor, (char *) "1", 1) == 1){
                printf("Sent: message 1\n");
            }
            else{
                printf("Error sending command\n");
            }
            close_frdm_connection(file_descriptor);
            break;
        case 2:
            //
            break;
        default:break;
    }
}



#endif
//======================================================

#ifndef MAIN_AM

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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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

void ReadCommands(int parsed_command);
void ProcessComands(int command);
void stringToHex(unsigned char* hexString, const char* string, int stringLenght);
void hexToString(char* string, unsigned char* hexString, int stringLenght);

int frame_data(const char * send_data, char* frame_data, unsigned int* frame_data_length);
int get_data (char* frame, unsigned int frame_length, char* data, unsigned int* data_length);

void start_primary_station();
void start_secondary_station();
void test_hdlc_frdm();
void frdm_log();
void error(const char *msg);

int main(int argc, char* argv[])
{
    int command = -1;
    if(argc == 2){
        command = (int)(*argv[1]) - 48;
    }
    ReadCommands(command);

    return 0;
}

//======================================================
// Implementacion de operaciones auxiliares

void ReadCommands(int parsed_command){
    printf("\n\n******************************************************\n");
    printf("******************************************************\n");
    printf("Comandos:\n");
    printf("\t\"0\"-> Quit\n");
    printf("\t\"1\"-> Toggle LEDs\n");
    printf("\t\"2\"-> Test encode and decode data with yahdlc\n");
    printf("\t\"3\"-> Start primary station for HDLC communication\n");
    printf("\t\"4\"-> Start secondary station for HDLC communication\n");
    printf("\t\"4\"-> Test HDLC \n");
    printf("\n");
    int command;
    if(parsed_command > 0){
        ProcessComands(parsed_command);
    }
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
            if(send_to_frdm(file_descriptor, (char *) "1", 1) == 1){
                printf("Sent: message 1\n");
            }
            else{
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
        case 6:
            frdm_log();
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

void frdm_log() {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    //if (argc < 3) {
//        fprintf(stderr,"usage %s hostname port\n", argv[0]);
//        exit(0);
//    }
    portno = 5000;//atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("192.168.1.52");//gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
    }
    while(true) {
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0){
            error("ERROR reading from socket");
            break;
        }
        printf("%s\n",buffer);
    }
    close(sockfd);
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

#endif
//======================================================
