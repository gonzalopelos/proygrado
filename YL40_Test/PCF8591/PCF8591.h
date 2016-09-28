/* PCF8591 - I2C 8 bit, 4 channel A/D and 8 bit, 1 channel D/A converter
 * Copyright (c) 2013 Wim Huiskamp
 *
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * version 0.2 Initial Release
*/
#ifndef _PCF8591_H
#define _PCF8591_H

/** Driver for PCF8591 I2C I2C A/D and D/A converter
 *
 * @code
 * #include "mbed.h"
 * #include "PCF8591.h"
 * 
 * // I2C Communication
 * I2C i2c_adcdac(p28,p27); // SDA, SCL for LPC1768
 * //I2C i2c_adcdac(P0_10,P0_11); // SDA, SCL for LPC812
 *
 * //Declare a composite ADC and DAC device that may be used through public methods
 * PCF8591 adc_dac(&i2c_adcdac); // I2C bus, Default PCF8591 Slaveaddress
 *
 * //Declare independent ADC and DAC objects that may be used similar to mbed AnalogIn and AnalogOut pins
 * //PCF8591_AnalogOut anaOut(&i2c_bus);
 * //
 * //PCF8591_AnalogIn anaIn(&i2c_bus, PCF8591_ADC0);
 *
 * 
 * int main() {
 *   uint8_t count = 0; 
 *   uint8_t analog;  
 *
 *   while(1) {
 *     wait(0.2);    
 *     count++;       
 *
 *     // Composite device methods    
 *     adc_dac.write(count);                 // write D/A value
 *     analog = adc_dac.read(PCF8591_ADC0);  // read A/D value for Channel 0
 * 
 *     // mbed pin type methods    
 *     //anaOut = (float)count / 255.0;        // write D/A value using range 0.0f .. 1.0f
 *     //analog = anaIn * 33.0;                // read A/D value for Channel 0 in (Volts*10)
 *   }
 *  
 * }
 * @endcode
 */


//Address Defines for PCF8591
#define PCF8591_SA0 0x90
#define PCF8591_SA1 0x92
#define PCF8591_SA2 0x94
#define PCF8591_SA3 0x96
#define PCF8591_SA4 0x98
#define PCF8591_SA5 0x9A
#define PCF8591_SA6 0x9C
#define PCF8591_SA7 0x9E

//Register Defines for PCF8591
#define PCF8591_CTRL 0x00
#define PCF8591_ADR0 0x01
#define PCF8591_ADR1 0x02
#define PCF8591_ADR2 0x03
#define PCF8591_ADR3 0x04
#define PCF8591_DAR  0x01

//Control Register Defines for PCF8591
//AD Channels
#define PCF8591_ADC0 0x00
#define PCF8591_ADC1 0x01
#define PCF8591_ADC2 0x02
#define PCF8591_ADC3 0x03
//AD Channel Mask
#define PCF8591_CHMSK   0x03

//Auto Increment flag (0=disable, 1=enable)
#define PCF8591_AIF  0x04

//AD Channels Input modes
//4 Single Channels (Chan0=AIN0, Chan1=AIN0, Chan2=AIN0, Chan3=AIN0)
#define PCF8591_4S   0x00
//3 Diff Channels (Chan0=AIN0-AIN3, Chan1=AIN1-AIN3, Chan2=AIN2-AIN3)
#define PCF8591_3D   0x10
//2 Single Channels (Chan0=AIN0, Chan1=AIN1)
//1 Diff Channel    (Chan2=AIN2-AIN3)
#define PCF8591_2S_1D 0x20
//2 Diff Channels (Chan0=AIN0-AIN1, Chan1=AIN2-AIN3)
#define PCF8591_2D   0x30
//ChannelMode Mask
#define PCF8591_CHMDMSK  0x30

//Analog Output enable flag (0=disable, 1=enable)
#define PCF8591_AOUT 0x40

//Default Mode: ADC0, AutoIncr Off, 4 Single Chan, AnalogOut On
#define PCF8591_CTRL_DEF (PCF8591_ADC0 | PCF8591_4S | PCF8591_AOUT)


/** Create a PCF8591 object connected to the specified I2C bus and deviceAddress
 *
*/
class PCF8591 {
public:
  /** Create a PCF8591 AD and DA object using a specified I2C bus and slaveaddress
   *
   * @param I2C &i2c the I2C port to connect to 
   * @param char deviceAddress the address of the PCF8591
  */  
  PCF8591(I2C *i2c, uint8_t deviceAddress = PCF8591_SA0);
  
#if(0)
//These methods are disabled since they interfere with normal expected behaviour for mbed type Analog I/O
//
  /** Set ADC mode
   *
   * @param adc_mode      ADC Channel mode, valid Range (PCF8591_4S, PCF8591_3D, PCF8591_2S_1D, PCF8591_2D)
  */  
  void setADCMode(uint8_t mode);

  /** Set DAC mode
   *
   * @param dac_mode      DAC Channel mode, valid Range (false=disable, true=enable)
  */  
  void setDACMode(bool mode);
#endif

/** Write Analog Output
  *
  * @param  analogOut  output value (0..255)
  */  
  void write(uint8_t analogOut);        

/** Read Analog Channel 
  *
  * @param Channel   ADC channel, valid range PCF8591_ADC0, PCF8591_ADC1, PCF8591_ADC2 or PCF8591_ADC3
  * @return value    uint8_t AD converted value (0..255, representing 0V..Vcc)   
  */  
  uint8_t read(uint8_t channel);


  
protected:
  I2C *_i2c;                    //I2C bus reference
  uint8_t _slaveAddress;        //I2C Slave address of device
  uint8_t _mode;                //Device mode  

/** Initialise AD and DA driver 
  *
  */  
  void _init(); 
};


/** Create a PCF8591 AnalogOut object connected to the specified I2C bus and deviceAddress
 *
*/
class PCF8591_AnalogOut {
public:
  /** Create a PCF8591 Analogout object using a specified I2C bus and slaveaddress
   *
   * @param I2C &i2c the I2C port to connect to 
   * @param char deviceAddress the address of the PCF8591
  */  
  PCF8591_AnalogOut(I2C *i2c, uint8_t deviceAddress = PCF8591_SA0);

  void write(float value);
  #ifdef MBED_OPERATORS
  /** An operator shorthand for write()
   */ 
  PCF8591_AnalogOut& operator= (float value);
  #endif

protected:
  I2C *_i2c;                    //I2C bus reference
  uint8_t _slaveAddress;        //I2C Slave address of device
  uint8_t _mode;                //Device mode  

/** Initialise DAC driver 
  *
  */  
  void _init(); 
};


/** Create a PCF8591 AnalogIn object connected to the specified I2C bus and deviceAddress
 *
*/
class PCF8591_AnalogIn {
public:
  /** Create a PCF8591 AnalogIn object using a specified I2C bus and slaveaddress
   *
   * @param I2C &i2c the I2C port to connect to 
   * @param channel ADC channel of the PCF8591   
   * @param char deviceAddress the address of the PCF8591
  */  
  PCF8591_AnalogIn(I2C *i2c, uint8_t channel, uint8_t deviceAddress = PCF8591_SA0);

  float read();
  #ifdef MBED_OPERATORS
  /** An operator shorthand for read()
   */ 
  operator float();
  #endif

protected:
  I2C *_i2c;                    //I2C bus reference
  uint8_t _slaveAddress;        //I2C Slave address of device
  uint8_t _mode;                //Device mode  
  uint8_t _channel;             //Channel    

/** Initialise DAC driver 
  *
  */  
  void _init(); 
};


#endif