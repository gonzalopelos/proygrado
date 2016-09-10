#include "mbed.h"

Serial pc(USBTX, USBRX);
DigitalOut led(LED_RED);
DigitalOut ledGreen(LED_GREEN);
I2C i2c(I2C_SDA, I2C_SCL);

#define DAC_ENABLE 0x40

int main()
{
	led = 0;
	ledGreen = 0;

	int addrDAC = 0x48 << 1;
	int ack = 0;
	char i2c_cmd[2];
	i2c_cmd[0] = DAC_ENABLE;
	while (true){
		for (uint i = 1; i < 256; i++){
			i2c_cmd[1] = (uint8_t)i;
			ack = i2c.write(addrDAC, i2c_cmd, 2);
			if (ack  ==  0){
				led = 1;
				ledGreen = 0;
			}
			else{
				led = 0;
				ledGreen = 1;
			}
		}

		led = 1;
		ledGreen = 0;
		for (int i = 255; i > 0; i--){
			i2c_cmd[1] = (uint8_t)i;
			ack = i2c.write(addrDAC, i2c_cmd, 2);
			if (ack  ==  0){
				led = 1;
				ledGreen = 0;
			}
			else{
				led = 0;
				ledGreen = 1;
			}
		}
	}
}
