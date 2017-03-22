#ifndef MCP_4728_H
#define MCP_4728_H
#include "getVersion.h"

#include "stdbool.h"


#include "dev_interface_def.h"
#include "I2CInterface.h" 
#include "DACInterface.h" 

#define VERSION_MCP4728_HDR "0.50"

/** MCP4728 class.
 *  Used for interfacing with a mcp4728 12-Bit QUAD  Digital-to-Analog Converter.
 *  For version 0.4X mode can not be set ( Intern ref,  gain x2  power mode normal) 
 *  It has to be used with the  https://developer.mbed.org/users/wbeaumont/code/DevInterfaces/ package 
 *  This includes the "virtual" I2CInterface class that is the interface to the I2C device 
 *  An implementation of the I2Cinterface class for the MBED can be found at 
 * 
 *  ( DACInterface inherits getVersion  )
 *  ee-prom set / read not tested / implemented. 
 *
 *  V 0.50 added methods to set the modes ( gain, ch power mode , reference 
 *
 * (C) Wim Beaumont Universiteit Antwerpen 2015  2016
 *  
 */
class MCP4728 : public DACInterface {
   
    public:
    /** The device supports two types of power modes: normal and power-down. In normal mode the device
     * operates a normal digital to analog conversion. In power-down mode all digitial to analog  
     * conversion is stopped, resulting in the device using less power (typically 60nA). Also, in power 
     * down mode Vout will be pulled to ground using either a 1k, 100k or 500k ohm internal resistors. */
    enum PowerMode {
        /** In normal mode the device operates a normal D2A conversion. */
        Normal=0,
        /** Enter the device into a power down mode, and pull Vout to ground using an internal 1k resistor. */
        PowerDown1k=1,
        /** Enter the device into a power down mode, and pull Vout to ground using an internal 100k resistor. */
        PowerDown100k=2,
        /** Enter the device into a power down mode, and pull Vout to ground using an internal 500k resistor. */
        PowerDown500k=3
    };
    /** Voltage reference can be either External, this means the Vdd or internal , 2.048 V 
        The output goes from 0 to Vref ( either 2.048 V or Vdd ), 
    */
    enum VrefMode {
        ExternRef=0,  // == VDD 
        InternRef=1    
    };
    /** In case the internal reference is used the output can be multiplied by a factor 2. 
        in that case the output goes between 0 and 4.096 V 
    */ 
    enum GainMode {
        GainX1=0,
        GainX2=1
    };
    
    
    
        
    
    /** The device supports 3 different I2C bus frequencies.*/
    enum BusFrequency {
        /** Standard 100kHz bus. */
        Standard100kHz,
        /** Fast 400kHz bus. */
        Fast400kHz,
        /** High Speed 3.4Mhz bus. WARNING: the test suite fails for the mbed LPC1768 when this frequency is selected - not tested on other platforms.*/
        HighSpeed3_4Mhz
    };
    /** structure used to set the channel configuration */
    typedef struct MCP4728ChCfg{
         GainMode gain;
         VrefMode  vref;
         PowerMode pwr;
    } MCP4728ChCfgdummy;

    
    /** Create an mcp4725 I2C interface
     * 
     * @param i2cinterface pointer to the   I2C interface (proto type) 
     * @param device_address_bits The 3bit address bits of the device.
     * @param Vdd the voltage on the Vdd pin ( power) is used to calculate voltage is Vref is set to external 
     */
    MCP4728(I2CInterface* i2cinterface,  int device_address_bits, float Vdd );
    
      
    /** Read the contents of the dac registers,( also eeprom settings)  , and if an eeprom write is currently active.
     *  result of the reading is stored in the channelstatus  structure .
     * @param checkall  check if the channel nr and adress is returned correctly in 04x this is not tested /checked so  set to false .     
     * @returns  0 on success non-0 on failure
     */
     int update(bool checkall);   

    /** Read the contents of the dac registers,( also eeprom settings)  , and if an eeprom write is currently active.
     *  result of the reading is stored in the channelstatus  structure .
     * implementation of the update from the DACInterface
     * @returns  0 on success non-0 on failure
     */
      
     virtual int update() { return update(false);}
     
     
    /** format a string with the channel status 
    * @param ch :  the channel nr
    * @param updatereq: if true the function update(true) is called else the class status table is used . 
    * @param eprom: if true the values in the eprom are used  else the values of the DAC channel 
    * @param str a pointer to an array of char  by example chanstatstr[200] 200 is minuimum.
    * @param dimension of the str array  ( so sizeoff(chanstatstr)  
    * @return the number of chars in the string or negative in case of an error 
    */
    int StatusString(int ch,bool updatereq ,bool eprom,  char *str , int length);
    
    /**gives the last value of the DAC register obtained by the last update  command.
        Implementation of the getDACvalue of the DACInterface 
       @param value by reference , the DAC value for the channel
       @param ch  the channel nr 0..3  
       @return the I2C result of the last update command 
    */
    virtual int getDACvalue(int &value, int ch=0);
   /** gives the last value of the DAC output voltage obtained by the last update  command.
       Use the channel status bits and eventual Vdd value  to convert  the DAC value to a voltage.
       Implementation of the getVoltagevalue of the DACInterface 
       @param voltage by reference , the DAC output voltage for the channel
       @param ch  the channel nr 0..3  
       @return the I2C result of the last update command 
    */
    virtual int getVoltage(float &voltage, int ch=0);
    
    /** set the dac value for a certain channel. 
        Implementation of the setDACvalue of the DACInterface 
        It also sets the DAC configuration bits  ( Vref, power mode , gain ) 
        ( For version 0.4x  this is internal, normal power , gain x2 ) 
        @param value  the DAC value to be set 
        @param ch  the channel nr 0..3  
        @return the I2C result when trying to set the DAC value
    */
    virtual int setDACvalue( int value, int ch=0);
    
    
    /** gives the last value of the DAC output voltage obtained by the last update  command.
       Use the channel configuration setings and eventual Vdd value  to convert the voltage to a DAC value.
       Then it calls setDACValue  so it sets also the channel configuration. 
       Implementation of the setVoltagevalue of the DACInterface 
       @param voltage by reference , the request  DAC output voltage for the channel
       @param ch  the channel nr 0..3  
       @return the I2C result when trying to set the DAC value
    */
    
    virtual int setVoltage (float voltage, int ch=0);
    
    /**  prepare power mode to 1k for all channels , mode becomes active with the next set voltage for the channel
    */
    void setPowerMode1k(){ for ( int ch=0;ch<4;ch++) ChCfg[ch].pwr=PowerDown1k;}
    
    /**  prepare power mode to 1k for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the power mode has to be set for. 
    */
    void  setPowerMode1k(int ch ){  ChCfg[ch].pwr=PowerDown1k;}
    
    /**  prepare power mode to 100k for all channels , mode becomes active with the next set voltage for the channel
    */
    void  setPowerMode100k(){ for ( int ch=0;ch<4;ch++) ChCfg[ch].pwr=PowerDown100k;}
    
    /**  prepare power mode to 100k for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the power mode has to be set for. 
    */
    void  setPowerMode100k(int ch ){  ChCfg[ch].pwr=PowerDown100k;}

    /**  prepare power mode to 500k for all channels , mode becomes active with the next set voltage for the channel
    */
    void  setPowerMode500k(){ for ( int ch=0;ch<4;ch++) ChCfg[ch].pwr=PowerDown500k;}
    
    /**  prepare power mode to 500k for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the power mode has to be set for. 
    */
    void  setPowerMode500k(int ch ){  ChCfg[ch].pwr=PowerDown500k;}

    /**  prepare power mode to normal (active) for all channels , mode becomes active with the next set voltage for the channel
    */
    void  setPowerModeNormal(){ for ( int ch=0;ch<4;ch++) ChCfg[ch].pwr=Normal;}
    
    /**  prepare power mode to normal (active)  for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the power mode has to be set for. 
    */
    void setPowerModeNormal(int ch ){  ChCfg[ch].pwr=Normal;}

    /**  prepare to intern reference for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the power mode has to be set for. 
    */
    void setRefIntern ( ) { for ( int ch=0;ch<4;ch++)  ChCfg[ch].vref=InternRef;}
    
    /**  prepare to intern reference  for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the reference  has to be set for. 
    */
    void setRefIntern (int ch ) {   ChCfg[ch].vref=InternRef;}

    /**  prepare to extern reference for a channel , mode becomes active with the next set voltage for the channel
        @param the channel the power mode has to be set for. 
    */
    void setRefExtern ( ) { for ( int ch=0;ch<4;ch++)  ChCfg[ch].vref=ExternRef;}

    /**  prepare to intern reference  for a channel. 
        mode becomes active with the next set voltage for the channel!!
        @param the channel the reference  has to be set for. 
    */
    void setRefExtern (int ch ) {  ChCfg[ch].vref=ExternRef;}
    
    /**  prepare to set the output gain to 2 for all channel 
        mode becomes active with the next set voltage for the channel!!
        @param the channel the power mode has to be set for. 
    */
    void setGainX2 () {for ( int ch=0;ch<4;ch++)   ChCfg[ch].gain=GainX2;}
    
    /**  prepare to set the output gain to 2  for a channel 
        mode becomes active with the next set voltage for the channel!!
        @param the channel the reference  has to be set for. 
    */
    void  setGainX2 (int ch) {  ChCfg[ch].gain=GainX2;}
    
    /**  prepare to set the output gain to 1 for all channel 
        mode becomes active with the next set voltage for the channel!!
        @param the channel the power mode has to be set for. 
    */
    void setGainX1 () {for ( int ch=0;ch<4;ch++)   ChCfg[ch].gain=GainX1;}
    
    /**  prepare to set the output gain to 1  for a channel 
        mode becomes active with the next set voltage for the channel!!
        @param the channel the reference  has to be set for. 
    */
    void setGainX1 (int ch) {  ChCfg[ch].gain=GainX1;}
  
    
    protected:
    /** pointer to the I2C interface driver. */
    I2CInterface* _i2c_interface;
    /** The full i2c device address. */
    int _device_address;
    char _adrbytes;
    /** calculates the voltage based on the value and the channel configuration bits 
        @param value : the value of the DAC register 
        @param gain  : the gain setting  x1 or x2 
        @param vref  : use internal or external reference 
        @return   : the voltage based on the parameters 
    */
    float dig2volt( short int value,GainMode gain,  VrefMode  vref); 

    /** calculates the nearest DAC register setting to get the requested voltage output 
        given the DAC configuration.  There is no "under/ overflow"  check
        @param  voltage the requested voltage
        @param gain  : the gain setting  x1 or x2 
        @param vref  : use internal or external reference 
        @return   : the DAC value based  the parameters to get the requested output voltage 
    */
    short int volt2dig( float voltage,GainMode gain,  VrefMode  vref);

    /** holds the Vdd value , used to calculate the voltage in case of vref = external */
    float Vdd;
    /** holds the  status of the last update request */
    int lastupdateresult;
    
    /** array for reading the status will be removed after debugging so don't use for future development */
    char rbdata[24] ; 

    /** used setting the configuration bits for each channel
        These values are the "requested" values so not the actual values. 
        These values are not updated after a update request !! 
     */
    MCP4728ChCfg ChCfg[4]; 
    /** structure to store all channel status / config bits */ 
    typedef struct MCP4728ChStatus{
         GainMode gain;
         VrefMode  vref;
         PowerMode pwr;
         bool busy;
         bool por;
         short int dacvalue;
         short int promvalue;
         GainMode promgain;
         VrefMode  promvref;
         PowerMode prompwr;
         
    } MCP4728Chstatus;
    
    /** storage of the channel status and configuration bits */ 
    MCP4728ChStatus chstat[4]; 
    
    
};

#endif