/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009
 
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif
//======================================================
 //Definicion de operaciones auxiliares

void ReadCommands(hid_device *handle);
void ProcessComands(int command, hid_device *handle);
void stringToHex(unsigned char* hexString, const char* string, int stringLenght);
void hexToString(char* string, unsigned char* hexString, int stringLenght);

 //======================================================

 

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[256];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;
	
	if (hid_init())
		return -1;

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

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;
	

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x04d8, 0x003f, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);
	
	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
	//res = hid_read(handle, buf, 17);

	// Send a Feature Report to the device
	/*
	buf[0] = 0x2;
	buf[1] = 0xa0;
	buf[2] = 0x0a;
	buf[3] = 0x00;
	buf[4] = 0x00;
	res = hid_send_feature_report(handle, buf, 17);
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	memset(buf,0,sizeof(buf));
*/
/*
	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}
*/

	

	// Request state (cmd 0x81). The first byte is the report number (0x1).
/*	buf[0] = 0;
	buf[1] = 0x81;
	hid_write(handle, buf, 17);
	if (res < 0)
		printf("Unable to write() (2)\n");

	usleep(500*10000);
*/	
	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
/*	res = 0;
	while (res == 0) {
		res = hid_read(handle, buf, sizeof(buf));
		if (res == 0)
			printf("waiting...\n");
		if (res < 0)
			printf("Unable to read()\n");
		#ifdef WIN32
		Sleep(500);
		#else
		usleep(500*1000);
		#endif
	}

	
	printf("Data read:\n   ");
	// Print out the returned buffer.
	for (i = 0; i < res; i++)
		printf("%02hhx ", buf[i]);
	printf("\n");

*/

	ReadCommands(handle);

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

//======================================================
// Implementacion de operaciones auxiliares

void ReadCommands(hid_device *handle){
 	printf("******************************************************\n");
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
 		ProcessComands(command, handle);
 	} while(command != 0);
 }

void ProcessComands(int command, hid_device *handle){
 	int res;
	int i;
 	unsigned char buf[256];
	memset(buf,0x00,sizeof(buf));
 	switch(command){
 		case 0: // Quit command
 			return;
 		break;
 		case 80: //Toggle LEDs command
 			// Toggle LED (cmd 0x80). The first byte is the report number (0x1).
			buf[0] = 0;
			buf[1] = 0x80;
			res = hid_write(handle, buf, 17);
			if (res < 0) {
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
 		break;
 		case 81:
 			memset(buf,0x00,sizeof(buf));
		 	buf[0] = 0;
			buf[1] = 0x81;
			res = hid_write(handle, buf, 17);
			if (res < 0)
				printf("Unable to write() (2)\n");

			usleep(500*10000);
			res = 0;
			while (res == 0) {
				res = hid_read(handle, buf, sizeof(buf));
				if (res == 0)
					printf("waiting...\n");
				if (res < 0)
					printf("Unable to read()\n");
				
				usleep(500*1000);
			}
			printf("Data read (%d bytes):\n", res);
			// Print out the returned buffer.
			for (i = 0; i < res; i++)
				printf("%02hhx ", buf[i]);
			printf("\n");
 		break;
 		case 86:
 			buf[0] = 0;
			buf[1] = 0x86;
			buf[2] = 'H';
			buf[3] = 'o';
			buf[4] = 'l';
			buf[5] = 'a';
			res = hid_write(handle, buf, 17);
			if (res < 0) {
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
			else {
				printf("%s\n", "Data sent successfully");
			}
		break;
		case 87:
			buf[0] = 0;
			buf[1] = 0x87;
			res = hid_write(handle, buf, 17);
			if (res < 0)
			{
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
			else{
				printf("%s: %d\n", "UART available to read", buf[1]);
			}
		break;
		case 88:
			buf[0] = 0;
			buf[1] = 0x88;
			res = hid_write(handle, buf, 17);
			if (res < 0)
			{
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
			else {
				printf("%s: %d\n", "UART readed", buf[2]);
			}
		break;
		case 100:
			buf[0] = 0;
			buf[1] = 0x9C;

			char text[100];
			strcpy((char *)text, "0000");
			unsigned char hexText[100];
			
			stringToHex(hexText, text, 4);
			
			strncpy((char*)buf+2, (char*)hexText, 4);

			res = hid_write(handle, buf, 17);
			if (res < 0){
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
			else{
				printf("%s\n", "Data sent successfully");

				usleep(500*10000);
				res = 0;
				while (res == 0) {
					res = hid_read(handle, buf, sizeof(buf));
					if (res == 0) {
						printf("waiting...\n");
					}
					else if (res < 0){
						printf("Unable to read()\n");
					}
					else {
						printf("Data read (%d bytes):\n", res);
						hexToString(text, buf, res);
					}
					usleep(500*1000);
				}
			}			
		break;
		case 101:
			buf[0] = 0;
			buf[1] = 0x9C;

			char text2[100];
			strcpy((char *)text2, "0000");
			unsigned char hexText2[100];
			
			stringToHex(hexText2, text2, 4);
			
			strncpy((char*)buf+2, (char*)hexText2, 4);

			res = hid_write(handle, buf, 17);
			if (res < 0){
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
			else{
				printf("%s\n", "Data sent successfully");
				usleep(500*10000);
			}
		break;
		case 102:
			res = 0;
			while (res == 0) {
				res = hid_read(handle, buf, sizeof(buf));
				if (res == 0) {
					printf("waiting...\n");
				}
				else if (res < 0){
					printf("Unable to read()\n");
				}
				else {
					printf("Data read (%d bytes):\n", res);
					hexToString(text, buf, res);
				}
				usleep(500*1000);
			}
		break;
		case 103:
			//buf[0] = 1;
			//buf[1] = 0x00;

			char text3[100];
			strcpy((char *)text3, "test");
			unsigned char hexText3[100];
			
			stringToHex(hexText3, text3, 4);
			
			strncpy((char*)buf, (char*)hexText3, 4);

			res = hid_write(handle, buf, 17);
			if (res < 0){
				printf("Unable to write()\n");
				printf("Error: %ls\n", hid_error(handle));
			}
			else{
				printf("%s\n", "Data sent successfully");
				usleep(500*10000);
			}
		break;
 		default: return;
 	}
 }

void stringToHex(unsigned char* hexString, char* string, int stringLenght){
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

 //======================================================