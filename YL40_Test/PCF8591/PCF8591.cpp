/* PCF8591 - I2C 8 bit 4 channel A/D and 8 bit 1 channel D/A converter
 * Copyright (c) 2013 Wim Huiskamp
 *
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * version 0.2 Initial Release
*/
#include "mbed.h"
#include "PCF8591.h"


/** Create a PCF8591 AD and DA object using a specified I2C bus and slaveaddress
  *
  * @param I2C &i2c the I2C port to connect to 
  * @param char deviceAddress the address of the PCF8591
  */  
PCF8591::PCF8591(I2C *i2c, uint8_t deviceAddress) : _i2c(i2c) {
    
  _slaveAddress = deviceAddress;
  _init(); 
}


#if(0)
//These methods are disabled since they interfere with normal expected behaviour for mbed type Analog I/O
//
  
/** Set ADC mode
  *
  * @param adc_mode      ADC Channel mode, valid Range (PCF8591_4S, PCF8591_3D, PCF8591_2S_1D, PCF8591_2D)
  */  
void PCF8591::setADCMode(uint8_t mode){
  uint8_t data[6];
  
  _mode = _mode & ~PCF8591_2D; // Clear ADC mode bits
  
  switch (mode) {
    case PCF8591_4S:
                _mode = _mode | PCF8591_4S;
                break; 
    case PCF8591_3D:
                _mode = _mode | PCF8591_3D;
                break;    
    case PCF8591_2S_1D:
                _mode = _mode | PCF8591_2S_1D;
                break;    
    case PCF8591_2D:
                _mode = _mode | PCF8591_2D;
                break;    
    default:
                _mode = _mode | PCF8591_4S;    
                break; 
  }    
        
  data[0] = _mode;             // Control Reg
  
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 1);  
   
};

/** Set DAC mode
  *
  * @param dac_mode      DAC Channel mode, valid Range (false=disable, true=enable)
  */  
void PCF8591::setDACMode(bool mode) {
  uint8_t data[6];
   
  if (mode) 
    _mode = _mode | PCF8591_AOUT;  // Enable DAC output
  else
    _mode = _mode & ~PCF8591_AOUT; // Disable DAC output

  data[0] = _mode;             // Control Reg
  
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 1);  
  
};
#endif


/** Write Analog Output
  *
  * @param  analogOut  output value (0..255)
  */  
void PCF8591::write(uint8_t analogOut) {
  uint8_t data[6];
   
  data[0] = _mode;             // Control Reg
  data[1] = analogOut;         // DAC Channel
  
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 2);  
};        

/** Read Analog Channel 
  *
  * @param Channel   ADC channel, valid range PCF8591_ADC0, PCF8591_ADC1, PCF8591_ADC2 or PCF8591_ADC3
  * @return value    uint8_t AD converted value (0..255, representing 0V..Vcc)   
  */  
#if(0)
// I2C block operations test
// Causes problems with Ticker and printf on LPC812. No issues found on LPC1768
uint8_t PCF8591::read(uint8_t channel) {
//  uint8_t data[6];
  uint8_t data[6] = {123,123,123,123,123,123}; //test to check failed read
    
  _mode   = _mode & ~PCF8591_CHMSK;            // Clear ADC Channel bits
  _mode   = _mode | (channel & PCF8591_CHMSK); // Set ADC Channel bits
    
  data[0] = _mode;             // Init Control Reg
  
  // write data to the device to select the ADC channel
  _i2c->write(_slaveAddress, (char*) data, 1);  
 
  // read selected ADC channel
  // note that first byte is a 'dummy' and should be ignored
  _i2c->read(_slaveAddress, (char*) data, 2);  
  return data[1];
};

#else
// I2C single byte operations test
// No issues found on LPC812
uint8_t PCF8591::read(uint8_t channel) {
  uint8_t data[6];
//  uint8_t data[6] = {123,123,123,123,123,123}; //test to check failed read
    
  _mode   = _mode & ~PCF8591_CHMSK;            // Clear ADC Channel bits
  _mode   = _mode | (channel & PCF8591_CHMSK); // Set ADC Channel bits
    
  data[0] = _mode;             // Init Control Reg
  
  // write data to the device to select the ADC channel
  _i2c->start();  
  _i2c->write(_slaveAddress);        // Slave write address 
  _i2c->write(data[0]);  
  _i2c->stop();  
   
  // read selected ADC channel
  // note that first byte is a 'dummy' and should be ignored
  _i2c->start();  
  _i2c->write(_slaveAddress | 0x01); // Slave read address 
  data[0]=_i2c->read(1);  //ack
  data[1]=_i2c->read(0);  //nack  
  _i2c->stop();  
  
  return data[1];
};

#endif


/** Initialise AD and DA driver 
  *
  */  
void PCF8591::_init() {
  uint8_t data[6];
  
  _mode   = PCF8591_CTRL_DEF;  // Init mode
  
  data[0] = _mode;             // Init Control Reg
  data[1] = 0x00;              // Init DAC Channel to 0
  
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 2);  
}; 





/** Create a PCF8591 AnalogOut object connected to the specified I2C bus and deviceAddress
 *
 */
PCF8591_AnalogOut::PCF8591_AnalogOut(I2C *i2c, uint8_t deviceAddress) : _i2c(i2c) {
    
  _slaveAddress = deviceAddress;
  _init(); 
}
 

/** Write Analog Output
  *
  * @param  analogOut value (0 .. 1.0)
  */  
void PCF8591_AnalogOut::write(float analogOut) {
  uint8_t data[6];

  data[0] = _mode;             // Control Reg

  // DAC Channel
  if (analogOut < 0.0) data[1] = 0;
  else if (analogOut >= 1.0) data[1] = 255;
       else data[1] = uint8_t (analogOut * 255.0);      
   
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 2);  
};        

/** Operator = Analog Output
  *
  * @param  analogOut value (0.0f .. 1.0f)
  */  
PCF8591_AnalogOut& PCF8591_AnalogOut::operator= (float value){  
  write(value);     
  return *this;   
}
 

/** Initialise DAC driver 
  *
  */  
void PCF8591_AnalogOut::_init() {
  uint8_t data[6];
  
  _mode   = PCF8591_CTRL_DEF;  // Init mode
  
  data[0] = _mode;             // Init Control Reg
  data[1] = 0x00;              // Init DAC Channel to 0
  
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 2);  
}; 



/** Create a PCF8591 AnalogIn object connected to the specified I2C bus and deviceAddress
 *
 */
PCF8591_AnalogIn::PCF8591_AnalogIn(I2C *i2c, uint8_t channel, uint8_t deviceAddress) : _i2c(i2c) {
    
  _slaveAddress = deviceAddress;
  _channel = channel;  
  _init(); 
}
 

/** Read Analog input
  *
  * @return analogIn value (0 .. 1.0)
  */  
float PCF8591_AnalogIn::read() {
  uint8_t data[6];
    
  data[0] = _mode;             // Init Control Reg
  
  // write data to the device to select the ADC channel
  _i2c->start();  
  _i2c->write(_slaveAddress);        // Slave write address 
  _i2c->write(data[0]);  
  _i2c->stop();  
   
  // read selected ADC channel
  // note that first byte is a 'dummy' and should be ignored
  _i2c->start();  
  _i2c->write(_slaveAddress | 0x01); // Slave read address 
  data[0]=_i2c->read(1);  //ack
  data[1]=_i2c->read(0);  //nack  
  _i2c->stop();  
  
 
  // ADC Channel
  return ((float) data[1] / 255.0);       
  
};


/** Operator float Analog Input
  *
  * @return analogIn value (0 .. 1.0)
  */  
PCF8591_AnalogIn::operator float(){
  return (read());    
}
 

/** Initialise ADC driver 
  *
  */  
void PCF8591_AnalogIn::_init() {
  uint8_t data[6];
  
  _mode   = PCF8591_CTRL_DEF;  // Init mode
  _mode   = _mode & ~PCF8591_CHMSK;             // Clear ADC Channel bits
  _mode   = _mode | (_channel & PCF8591_CHMSK); // Set ADC Channel bits
      
  data[0] = _mode;             // Init Control Reg
  
  // write data to the device
  _i2c->write(_slaveAddress, (char*) data, 1);  
}; 
