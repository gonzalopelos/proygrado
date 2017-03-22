#include "getVersion.h"

#define GETVERSION_SRC_VER  "0.20"
#ifdef MBED 
#include "mbed.h"
#else 
#include  <stdlib.h> 
#include  <stdio.h> 
#endif 

#define NULL 0


getVersion::getVersion(const char* ver_h,const char* ver_s, const char* time,const char* date) {
        sver=ver_s; hver=ver_h; ctime=time;cdate=date;
        sprintf(infostr,"HDR ver:  %s, SRC ver: %s",sver,hver);
        
        } ;

getVersion::getVersion(){sver=0; hver=0; ctime=0;cdate=0;
            sprintf(infostr,"HDR ver:  %s, SRC ver: %s",GETVERSION_HDR_VER,GETVERSION_SRC_VER);
};     


void getVersion::get_dec_version( unsigned short  hexversion , unsigned char & version, unsigned char& subversion) {
        subversion =(unsigned short)(  hexversion & 0xFF);
        hexversion=hexversion >> 8; 
        version =(unsigned char)(  hexversion & 0xFF);
    }



unsigned short  getVersion::get_hex_version_nr(const char * vers){    
    float verf = strtof(vers,NULL);
    unsigned short version=(unsigned short) verf;
    verf=verf-version;
    version = version<<8;
    verf=verf*100;
    unsigned char  sub=(unsigned char)verf;
    version=version| sub;
    return version;
}   






unsigned short getVersion::getHdrVersion(){ 
    if( hver == NULL) return  get_hex_version_nr(GETVERSION_HDR_VER);
    else return get_hex_version_nr(hver);
    }

    unsigned short getVersion::getSrcVersion(){
          if( sver == NULL) return  get_hex_version_nr(GETVERSION_SRC_VER);
    else return get_hex_version_nr(sver);
 }


unsigned short getVersion::getCompileTime(){
    if ( ctime == NULL ) return 0;
    else return 0;
}
    

unsigned short getVersion::getCompileDate(){
    if ( cdate == NULL ) return 0;
    else return 0;
}

unsigned short getVersion::getCompileYear(){
    
    if ( cdate == NULL ) return 0;
    else return 0;
}

char* getVersion::getversioninfo(){
    return infostr;
}
   
   
