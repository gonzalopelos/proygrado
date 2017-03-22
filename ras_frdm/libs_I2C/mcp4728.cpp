#include "mcp4728.h"
#include "mbed.h"


#define VERSION_MCP4728_SRC  "0.49"  

#define VREFVOLTAGE 2.048
#define CMDFAST  0

//#define CMDWRITEI2CADDR  3 // handled in a different class 
#define CMDWRITEVREF  4
#define CMDWRITEGAIN  6
#define CMDWRITEPWDDOWN  5

#define WRITEFCTMULTI  8  // write to multiple  DAC ch  given by ch selectio bits 
//#define WRITEFCTSEQ  2       // write to DAC AND eeprom,  from ch given in ch selection bits to D
//#define WRITEFCTSINGLE 3   // write to a singe DAC AND eeprom  ch given in ch selection bits

MCP4728::MCP4728(I2CInterface* i2cinterface, int device_address_bits , float Vddin ): 
    getVersion( VERSION_MCP4728_HDR,VERSION_MCP4728_SRC, __TIME__, __DATE__),_i2c_interface(i2cinterface) 
{
    _adrbytes= device_address_bits & 7;
    // Assemble the full I2C device address.
    _device_address = 0xC0; // Prime the full device address with the device code.
    _device_address |= (device_address_bits<<1);
    // next has to be read back from the device but
    Vdd=Vddin;
    for ( int ch=0;ch<4;ch++) {
             ChCfg[ch].pwr=Normal;
             ChCfg[ch].vref=InternRef;
             ChCfg[ch].gain=GainX2;
      }      
}

int MCP4728::getDACvalue(int& value, int ch){
    ch = ch & 7;
    value =chstat[ch].dacvalue;
    return lastupdateresult;
}
int  MCP4728::getVoltage(float &voltage, int ch){
    int value ; 
    int result = getDACvalue( value , ch ); 
    voltage = dig2volt( (short int) value, chstat[ch].gain,  chstat[ch].vref);
    return result;     
};

int MCP4728::setVoltage (float voltage, int ch){   
    short int value = volt2dig( voltage, ChCfg[ch].gain, ChCfg[ch].vref);
    return  setDACvalue( (int) value, ch);
        
};



    float MCP4728::dig2volt( short int value, GainMode gain,  VrefMode  vref ){
        float vout =(float)value / 4096;
        if ( vref == ExternRef ) {
            vout = Vdd * vout;
        }
        else {
            vout = VREFVOLTAGE * vout; 
            if ( gain == GainX2 ) vout = vout*2;
        }   
        return vout; 
    }
    short int MCP4728::volt2dig( float voltage, GainMode gain,  VrefMode  vref ){ 
        float tmp;
        tmp= 4096 * voltage;
        if ( vref == ExternRef ) {
          tmp= tmp/Vdd  ;                                
        }    
        else {
           if ( gain == GainX2 ) tmp=tmp/2;
           tmp = tmp /  VREFVOLTAGE;
        }
        return (short int) ( tmp +.5 );    
    }


int MCP4728::update(bool checkall ) {
   int result;
    
    // Read the raw data from the device.
    result = _i2c_interface->read(_device_address, rbdata, sizeof(rbdata)/sizeof(*rbdata), false);
    
    // Parse the raw data, extracting our fields. Refer to MCP4728 ref manual, section 6.2
    if (result == 0){
        int bytecnt=0;
        char chancheck=0;
        char addrcheck=0;
        for ( int cc =0 ; cc < 4 ; cc++) {
            bytecnt= cc*6; // 6 byte per channel
            chstat[cc].busy= (rbdata[bytecnt] & 0x80)? false:true;
            chstat[cc].por= (rbdata[bytecnt] & 0x40)? false:true;
            if (checkall) {
                chancheck=(rbdata[bytecnt] & 0x30) ;
                chancheck=chancheck >>4;  chancheck=chancheck & 3; 
                if ( cc != chancheck ) { result= chancheck ; result = result <<4 ; result=0xC |result | cc;} 
                addrcheck= (rbdata[bytecnt] & 0x07) ;
                if ( _adrbytes != addrcheck) { result= 0x08 | addrcheck ; result = result <<4 ; result=0x8 | result | cc;} 
            }
            bytecnt++; // point to the second addr 
            chstat[cc].vref = (enum VrefMode) ((rbdata[bytecnt] & 0x80)>>7);
            chstat[cc].pwr = (enum PowerMode) ((rbdata[bytecnt] & 0x06)>>5);
            chstat[cc].gain = (enum GainMode) ((rbdata[bytecnt] & 0x10)>>4);
            chstat[cc].dacvalue  = (rbdata[bytecnt++] & 0xF );// point to the third addr
            chstat[cc].dacvalue = chstat[cc].dacvalue << 8 ;
            chstat[cc].dacvalue = chstat[cc].dacvalue | rbdata[bytecnt++] ;// point to the forth addr
            // eprom bytes             
            chstat[cc].busy= (rbdata[bytecnt] & 0x80)? false:true;
            chstat[cc].por= (rbdata[bytecnt] & 0x40)? false:true;
            if (checkall) {
                chancheck=(rbdata[bytecnt] & 0x30) ;
                chancheck=chancheck >>4;  chancheck=chancheck & 3; 
                if ( cc != chancheck ) { result= chancheck ; result = result <<4 ; result=0xC |result | cc;} 
                addrcheck= (rbdata[bytecnt] & 0x07) ;
                if ( _adrbytes != addrcheck) { result= 0x08 | addrcheck ; result = result <<4 ; result=0x8 | result | cc;} 
            }
            bytecnt++; // point to the second addr 
            chstat[cc].promvref = (enum VrefMode) ((rbdata[bytecnt] & 0x80)>>7);
            chstat[cc].prompwr = (enum PowerMode) ((rbdata[bytecnt] & 0x06)>>5);
            chstat[cc].promgain = (enum GainMode) ((rbdata[bytecnt] & 0x10)>>4);
        
            chstat[cc].promvalue  = (rbdata[bytecnt++] & 0xF );
            chstat[cc].promvalue = chstat[cc].promvalue << 8 ;
            chstat[cc].promvalue = chstat[cc].promvalue | rbdata[bytecnt] ;       
        }
    }        
    lastupdateresult=result; 
    return result;
}


int MCP4728::setDACvalue( int value, int ch){
    char data[3], tmp ; 
 
    data[0] =  WRITEFCTMULTI << 3 ; // select write multi ch no eeprom 
    ch =ch & 3;
    tmp =(char)  ch << 1; // select the channel
    data[0] = data[0] | tmp ;
    data[0] = data[0] &  0xFE ; // update now , assuming !LDAC =0

    data[2] = 0xFF &  (char)value; // lower byte
    value=value >>8;
    data[1] = 0x0F &  (char)value;
    data[1] = data[1] | (char)( ChCfg[ch].gain <<4); //set gain
    data[1] = data[1] | (char)( ChCfg[ch].pwr <<5); // set pwr mode 
    data[1] = data[1] | (char)( ChCfg[ch].vref  <<7); // set pwr mode 

    //data[0]= 0x52; // 0b0101 0010  ch 1  !UDAC=0 
    //data[1]= 0x17; // 0b0000 0111   ref=Vdd, pwd =0  normal  ; gain =1  d11=0 ,111,       
    return _i2c_interface->write(_device_address, data, sizeof(data)/sizeof(*data), false);
 
    
}

// kept c compatible 
// no text interpretation of gain ,ref  and power mod 
int   MCP4728::StatusString(int ch,bool updatereq , bool eprom,  char *str , int length){
if (length < 200 ) return -200;
int result=0;
if ( updatereq ) result= update (false);
if ( result ) return result ;
ch=ch&3 ;

int dataidx= ch* 6 + 3* (int) eprom;
if ( eprom ) {
    
    
    result= sprintf( str, "d1:%02X d2:%02X  d3:%02X EEP Ch: %1d pwr: %1d Ref: %1d gain: %1d, value %04d", 
    (int)rbdata[dataidx++],(int)rbdata[dataidx++],(int)rbdata[dataidx],
    ch,(int) chstat[ch].prompwr, (int) chstat[ch].promvref , (int) chstat[ch].promgain, (int)  chstat[ch].promvalue );
    
}else {
    result= sprintf( str, "d1:%02X d2:%02X  d3:%02X DAC Ch: %01d pwr: %04d Ref: %04d gain: %04d, value %04d", 
    (int)rbdata[dataidx],(int)rbdata[dataidx+1],(int)rbdata[dataidx+2],
    ch,(int) chstat[ch].pwr, (int) chstat[ch].vref , (int) chstat[ch].gain, (int)  chstat[ch].dacvalue  );
}    
return result;     
}