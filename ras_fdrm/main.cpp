#include "mbed.h"

DigitalOut ledRed(LED_RED);
DigitalOut ledGreen(LED_GREEN);
I2C i2c(I2C_SDA, I2C_SCL);

#define SHARP_IR_ADDR 0x20

int main(){
	ledRed = 0;
	ledGreen = 0;
	wait(1);
	i2c.start();
	char readType[1];
	readType[0] = 3;
	wait(1);
	uint8_t ack = i2c.write(SHARP_IR_ADDR, readType, 1);
	i2c.stop();
	char i2c_dist[1];
	i2c.start();
	wait(1);
	while (true){
		i2c.read(SHARP_IR_ADDR, i2c_dist, 1);
		wait(0.5);

		if (i2c_dist[0]  >= 128){
			ledRed = 1;
			ledGreen = 0;
		}
		else{
			ledRed = 0;
			ledGreen = 1;
		}
	}
}

//#define DAC_ENABLE 0x40
//
//int main()
//{
//	wait(1);
//	ledRed = 0;
//	ledGreen = 0;
//
//	int addrDAC = 0x48 << 1;
//	int ack = 0;
//	char i2c_cmd[2];
//	i2c_cmd[0] = DAC_ENABLE;
//	while (true){
//		for (uint i = 1; i < 256; i++){
//			i2c_cmd[1] = (uint8_t)i;
//			ack = i2c.write(addrDAC, i2c_cmd, 2);
//			if (ack  ==  0){
//				ledRed = 1;
//				ledGreen = 0;
//			}
//			else{
//				ledRed = 0;
//				ledGreen = 1;
//			}
//		}
//
//		ledRed = 1;
//		ledGreen = 0;
//		for (int i = 255; i > 0; i--){
//			i2c_cmd[1] = (uint8_t)i;
//			ack = i2c.write(addrDAC, i2c_cmd, 2);
//			if (ack  ==  0){
//				ledRed = 1;
//				ledGreen = 0;
//			}
//			else{
//				ledRed = 0;
//				ledGreen = 1;
//			}
//		}
//	}
//}
