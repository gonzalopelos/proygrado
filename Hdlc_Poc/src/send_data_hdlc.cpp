/*******************************************************
 HIDAPI & HDLC
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>     // Error number definitions 
#include "usb_controller.h"
#include "yahdlc.h"





#define MAX_FRAME_PAYLOAD		512
#define HEADERS_LENGTH			8

#define TOTAL_FRAME_LENGTH		MAX_FRAME_PAYLOAD + HEADERS_LENGTH

#define BAUDRATE B38400            
/* change this definition for the correct port */
#define MODEMDEVICE "/dev/bus/usb/002/006"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

 #define MAX_STR 255

wchar_t wstr[MAX_STR];

volatile int STOP=FALSE;






//======================================================
 //Definicion de operaciones auxiliares

void ReadCommands();
void ProcessComands(int command);
void stringToHex(unsigned char* hexString, const char* string, int stringLenght);
void hexToString(char* string, unsigned char* hexString, int stringLenght);

int frame_data(const char * send_data, unsigned char* frame_data_hexa, unsigned int frame_data_length);



 

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
 	printf("\t\"80\"-> Toggle LEDs\n");
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

 	switch(command){
 		case 0: // Quit command
 			return;
 		break;
 		case 1: //Toggle LEDs command
 			struct hid_device_info * devs, *cur_dev;
 			devs = hid_enumerate(0x0, 0x0);
 			cur_dev = devs;	
			while (cur_dev) {
				printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
				printf("\n");
				printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
				printf("  Product:      %ls\n", cur_dev->product_string);
				printf("  Release:      %hx\n", cur_dev->release_number);
				printf("  Interface:    %d\n",  cur_dev->interface_number);
				printf("\n");
				cur_dev = cur_dev->next;
			}
			hid_free_enumeration(devs);
		break;
		case 2:
		    // Open the device using the VID, PID,
			// and optionally the Serial number.
			////handle = hid_open(0x4d8, 0x3f, L"12345");
			hid_device * usb;

			usb = hid_open(0x04d8, 0x003f, NULL);
			if (!usb) {
				printf("unable to open device\n");
		 		return;
			}

			// Read the Manufacturer String
			wstr[0] = 0x0000;
			res = hid_get_manufacturer_string(usb, wstr, MAX_STR);
			if (res < 0)
				printf("Unable to read manufacturer string\n");
			printf("Manufacturer String: %ls\n", wstr);

			// Read the Product String
			wstr[0] = 0x0000;
			res = hid_get_product_string(usb, wstr, MAX_STR);
			if (res < 0)
				printf("Unable to read product string\n");
			printf("Product String: %ls\n", wstr);

			// Read the Serial Number String
			wstr[0] = 0x0000;
			res = hid_get_serial_number_string(usb, wstr, MAX_STR);
			if (res < 0)
				printf("Unable to read serial number string\n");
			printf("Serial Number String: (%d) %ls", wstr[0], wstr);
			printf("\n");

			// Read Indexed String 1
			wstr[0] = 0x0000;
			res = hid_get_indexed_string(usb, 1, wstr, MAX_STR);
			if (res < 0)
				printf("Unable to read indexed string 1\n");
			printf("Indexed String 1: %ls\n", wstr);

			// Set the hid_read() function to be non-blocking.
			hid_set_nonblocking(usb, 1);

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

int frame_data(const char * send_data, unsigned char* frame_data_hexa, unsigned int frame_data_length)
{
	int ret;
	char frame_data[TOTAL_FRAME_LENGTH];
	unsigned int data_length = 0, frame_length = 0, seq_no = 0;
	yahdlc_control_t control;
	yahdlc_frame_t frame_type = YAHDLC_FRAME_DATA;

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
	ret = yahdlc_frame_data(&control, send_data, data_length, frame_data, &frame_length);

	/* If success */
	if (ret == 0)
	{
		//frame_data_hexa = unsigned char [frame_length];
		stringToHex(frame_data_hexa, send_data, frame_length);
		//return PyBytes_FromStringAndSize(frame_data, frame_length);
	// else
	// {
	// 	PyErr_SetString(PyExc_ValueError, "invalid parameter");
	// 	return NULL;
	}
	return ret;
}


 //======================================================