//#define MAIN_AM

#ifdef MAIN_AM
#include "mbed.h"
#include "PIDModule.h"
#include "dm3.h"

#define MOTORS_PER_CHASIS 2

//I2C i2c(PTE25, PTE24); // SDA, SCL para K64F
//PCF8591 DAC_I2C(&i2c, PCF8591_SA0); // I2C bus, Default PCF8591 Slaveaddress
//Serial pc(USBTX, USBRX);

DigitalOut led(LED_RED);
DigitalOut ledGreen(LED_GREEN);

int main()
{
	led = 1;
	ledGreen = 1;
	Dm3 *dm3 = Dm3::Instance();
	int power = 0;
	int i2cerror = 0;
	int chasis = 0;
	int motor = 0;
	while (true) {
		dm3->enable(1);
		chasis = 0;
		motor = 1;
		power = 0;
		i2cerror = dm3->motor_i2c(chasis*MOTORS_PER_CHASIS + motor, power);
		wait(2.0f);
		power = 50;
		i2cerror = dm3->motor_i2c(chasis*MOTORS_PER_CHASIS + motor, power);
		wait(2.0f);
		power = 100;
		i2cerror = dm3->motor_i2c(chasis*MOTORS_PER_CHASIS + motor, power);

		wait(2.0f);
	}
}

/*
 int main() {
 PIDModule pid[1];
 uint8_t count = 0;
 float vel = 0;
 while(1) {
 wait(0.1);
 count++;

 vel = pid[1].compute(0, 10);

 DAC_I2C.write(count);
 }
 }
 */

#endif

#ifndef MAIN_AM

#include "mbed.h"
#include "rtos.h"
<<<<<<< HEAD
#include "modules/Ultrasonic/Ultrasonic.h"
#include "modules/Mcc/Mcc.h"
#include "modules/Admin/Admin.h"
#include "modules/Motor/MotorModule.h"
#include "modules/DM3/Dm3Module.h"
=======
#include "modules/DM3/Dm3Security.h"
//#include "modules/Ultrasonic/Ultrasonic.h"
//#include "modules/Mcc/Mcc.h"
//#include "modules/Admin/Admin.h"
//#include "modules/Motor/MotorModule.h"
//#include "modules/DM3/Dm3Module.h"
>>>>>>> 96ebfe4c11b362ca11cb2d3187114ebd01ee2c9e
//#include "hdlc/frdm_communication.h"
//#include "modules/Logging/Logger.h"
//#include "modules/Ethernet/Communication.h"
//Ticker ticker_msg_rate;
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);

using namespace modules;
//Mcc mcc;

void heartbeat_task() {
	while (true) {
		led_green = !led_green;
		Thread::wait(500);
	}
}

//================================================
// TEST ULTRASONIC VARIABLES
<<<<<<< HEAD
DigitalOut trigger(PTA1);
DigitalIn echo(PTA2);
int distance = 0;
int correction = 0;
Timer sonar;
void dist(int distance)
{
	led_red = 0;
	printf("Distance changed to %d\r\n", distance);
}
Ultrasonic mu(PTA1, PTA2, .1, 1, &dist);
=======
//int _distance = 0;
//void dist(int distance)
//{
//	_distance = distance;
//}
//Ultrasonic mu(PTA1, PTA2, .1, 1, &dist);
>>>>>>> 96ebfe4c11b362ca11cb2d3187114ebd01ee2c9e
//================================================

int main() {

<<<<<<< HEAD
//	led_red = 1;
//	led_blue = 1;
//	led_green = 1;
//
//
//	wait(2);
////	mcc.send_message(0,0,"test",4);
=======
	led_red = 1;
	led_blue = 1;
	led_green = 1;
	Dm3Security dm3_security;
	Thread heartbeat(heartbeat_task);

//	wait(2);
>>>>>>> 96ebfe4c11b362ca11cb2d3187114ebd01ee2c9e
//	Admin admin_module;
//	MotorModule motorModule;
//	motorModule.init();
//	Dm3Module dm3Module;
//
//	Thread heartbeat(heartbeat_task);
//
//	while(1){
<<<<<<< HEAD
=======
//		mcc.send_message(0,1,"test",4);
//		wait(1);
>>>>>>> 96ebfe4c11b362ca11cb2d3187114ebd01ee2c9e
//		mcc.tick();
//	}

//==================================================
// ULTRASONIC LOGIC
<<<<<<< HEAD
	mu.startUpdates();//start mesuring the distance
	while(1)
	{
		mu.checkDistance();     //call checkDistance() as much as possible, as this is where
		wait(1);
	}
}
=======
//	mu.startUpdates();//start mesuring the distance
//	while(1)
//	{
////  	Do something else here
//		printf("the distance is: %dmm\n", _distance);
//		wait(1);
////		the class checks if dist needs to be called.
//	}
//}


//==================================================
//	Interruptions POC - START
//==================================================
//InterruptIn Interrupt(PTA1);
//
//
//void blink()
//{
//    wait(.4);
//    led_red=1;
//    led_blue=0;
//    wait(.4);
//    led_blue=1;
//    wait(.4);
//}
//
//int main()
//{
//	led_red = 1;
//	led_blue = 1;
//	led_green = 1;
//    Interrupt.fall(&blink);
//    led_blue=1;
//    while (1)
//    {
//        led_red=!led_red;
//        wait(.2);
//    }
//}
//==================================================
//	Interruptions POC - END
//==================================================
>>>>>>> 96ebfe4c11b362ca11cb2d3187114ebd01ee2c9e

}
#endif

