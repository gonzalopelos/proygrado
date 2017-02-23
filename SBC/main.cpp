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
#include <unistd.h>
#include <fstream>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <chrono>
#include <Communication.h>
#include "includes/Mcc.h"
#include "strings.h"
#include "includes/AdminModule.h"

#define MAX_FRAME_PAYLOAD		512
#define HEADERS_LENGTH			8

//======================================================
//Definicion de operaciones auxiliares

void ReadCommands(int parsed_command);
void ProcessComands(int command);
void frdm_log();
void error(const char *msg);
void test_tcp_connection();
void test_mcc();

Mcc mcc;

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

    switch(command){
        case 0: // Quit command
            return;
        case 1: //Toggle LEDs command
            break;
        case 2:
            char a;
            a = 't';
            char pepe[256];
            bzero(pepe, 256);
            pepe[strlen(pepe)] = a;
            printf("pepe: %s - length: %d\n", pepe, (int) strlen(pepe));
            a = 'e';
            pepe[strlen(pepe)] = a;
            printf("pepe: %s - length: %d\n", pepe, (int) strlen(pepe));
            a = 's';
            pepe[strlen(pepe)] = a;
            printf("pepe: %s - length: %d\n", pepe, (int) strlen(pepe));
            a = 't';
            pepe[strlen(pepe)] = a;
            printf("pepe: %s - length: %d\n", pepe, (int) strlen(pepe));

            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            frdm_log();
            break;
        case 7:
            test_tcp_connection();
            break;
        case 8:
            test_mcc();
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
    //exit(0);
}

void test_tcp_connection(){
    Communication *  communication_instance;
    int sent_messages = 0;
    int max_sent_messages = 100;
    int incomplete_messages_read = 0;
    long int bytes_sent = 0;
    long int bytes_read = 0;

    communication_instance = Communication::get_instance();
    typedef std::chrono::high_resolution_clock Clock;

    auto t1 = Clock::now();
    int n;
    char buffer[256];
    while (sent_messages<max_sent_messages) {
        int message_length = 0;
        while (sent_messages < max_sent_messages) {
            bzero(buffer, 256);
            message_length = sprintf(buffer, "0001020304050607080910111213141516171819202122232425262728293000010203040506070809101112131415161718");
            n = communication_instance->send_all(buffer, message_length);
            if (n == message_length) {
                //      printf("data sent successfully: %d\n", n);
                sent_messages ++;
            }
            bytes_sent += n;
            bzero(buffer, 256);
            n = communication_instance->receive(buffer, 256);
            if (n < 0) {
                error("ERROR reading from socket");
                break;
            } else if (n > 0) {
                //printf("Data read: %s\n", buffer);
                bytes_read += n;
                if(n<100){incomplete_messages_read++;}

            } else {
                printf("No data read\n");
            }
        }

    }

    auto t2 = Clock::now();
    long int nano_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    long double seconds = nano_seconds / (double)1000000000;

    printf("Sent (%d) messages in (%Lf) secconds\nSent (%ld) bytes --- read (%ld) bytes\nIncomplete messages read: %d\n", sent_messages, seconds, bytes_sent, bytes_read, incomplete_messages_read);
}

void test_mcc(){
    AdminModule admin_module;
//    mcc.send_message(0,0,"test", 4);

    while(1) {
        mcc.tick();
    }
}



#endif
//======================================================
