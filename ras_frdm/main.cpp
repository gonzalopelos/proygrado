#define MAIN_AM

#ifdef MAIN_AM
#include "mbed.h"
#include "PCF8591.h"
#define VERSION "22_07_2014"
#define CIBLE "KL25Z"


// I2C Communication
  I2C i2c_adcdac(PTC9,PTC8); // SDA, SCL for LPC1768
  //I2C i2c_adcdac(P0_10,P0_11); // SDA, SCL for LPC812
 
  //Declare a composite ADC and DAC device that may be used through public methods
  PCF8591 capteur0(&i2c_adcdac,PCF8591_SA0); // I2C bus, Default PCF8591 Slaveaddress
 
  //Declare independent ADC and DAC objects that may be used similar to mbed AnalogIn and AnalogOut pins
  //PCF8591_AnalogOut anaOut(&i2c_bus);
  //
  //PCF8591_AnalogIn anaIn(&i2c_bus, PCF8591_ADC0);
 
  
  int main() {
    uint8_t count = 0; 
    uint8_t analog;  
    printf("LAAS-CNRS ,PCF8591 ,%s ,%s\r\n",CIBLE,VERSION);
    while(1) {
      wait(1);    
      count++;       
 
      // Composite device methods    
     capteur0.write(count);                 // write D/A value
     analog = capteur0.read(PCF8591_ADC0);  // read A/D value for Channel 0
printf("AO=%d AI=%d \r\n",count,analog);
              // read A/D value for Channel 0 in (Volts*10)
  }

 
 }

#endif

#ifndef MAIN_AM

#include "mbed.h"
#include "hdlc/frdm_communication.h"
#include "rtos.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

//Serial pc(USBTX, USBRX);
//Serial pc2(USBTX, USBRX);

Thread hdlc_secondary_station_thread;
Thread pc_serial;
//Thread pc_serial_2;

void callback() {
    // Note: you need to actually read from the serial to clear the RX interrupt
//    pc.printf("%c\n", pc.getc());
//    if(pc.getc() == '1'){
//    	led1 = 1;
//    } else {
//    	led1 = 0;
//    }
//    //pc.printf("1-%c", pc.getc());
//    wait(0.5);

}

void callback2() {
    // Note: you need to actually read from the serial to clear the RX interrupt
//    pc.printf("%c\n", pc.getc());
    led1 = !led1;
   // pc2.printf("2-%s", pc2.getc());
    wait(3);

}

void pc_serial_echo() {
//	char data[20];
//	data[0] = '\0';
//	while(1) {
//		if(pc.readable()) {
//			sprintf(data, "FRDM %c\n", pc.getc());
//			pc.puts(data);
//			led1 = 0;
//		} else {
//			led1 = 1;
//		}
//		wait(1);
//	}
}

void pc_serial_2_echo() {
//	while(1) {
//		if(pc2.readable()) {
//			pc2.printf("2 %c\n", pc2.getc());
//			led2 = 0;
//		} else {
//			led2 = 1;
//		}
//		wait(1);
//	}
}


void hdlc_secondary_station() {
	led2 = 1;
	wait(1);
	led2 = 0;
//	hdlc_init(0x2);
}

int main() {
	led1 = 1;
	led2 = 1;
	led3 = 1;
	led4 = 1;

    //pc.attach(&callback);
    //pc2.attach(&callback2);
    //led1 = 1;

    //hdlc_secondary_station_thread.start(hdlc_secondary_station);
   // char command;
    //char data_read[256];
    //unsigned int data_read_size;
	//pc_serial.start(pc_serial_echo);
	//pc_serial_2.start(pc_serial_2_echo);
	int connection = open_frdm_connection();
	char data[256];
	int data_size;
	int data_sent_size;
    while (1) {
//        led1 = !led1;
        wait(0.5);
        //data_size = get_from_fdrm(connection, data, 256);
//        if(data_size > 0) {
//        	led1 = !led1;
//        	data_sent_size = send_to_frdm(connection, data, data_size);
//        	if(data_size == data_sent_size) {
//        		led3 = !led3;
//        	}
//        }

    }
}

#endif



