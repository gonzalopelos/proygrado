#ifndef GETVERSION_H
#define GETVERSION_H

/*  getVersion class
 *  to get version number ( compile date etc)  of a module
 *  to be used to verify module version used by a main program 
 *  can be used as inheritance class
 *  written for the SOLID SM1 control module firmware 
 *
 * author  Wim.Beaumont@Uantwerpen.be
 * (C)  Universiteit Antwerpen  5 October 2014 
 *
 * version history 
 * v 0.10  intial development vesion
 *  
 */

#define GETVERSION_HDR_VER  "0.20"



class getVersion {
 
 const char* hver;
 const char* sver;
 const char* ctime;
 const char* cdate;
 char infostr[100]; // 100 is not precise nr. 
 
public:
    getVersion(const char* ver_h,const char* ver_s=0, const char* time=0,const char* date=0) ;
    getVersion(); 
    
    // returns the version number of hdr of the  module in hex code     
    unsigned short getHdrVersion(); 
    // returns the version number of the  src of the  module  in hex code     
    unsigned short getSrcVersion(); 
    // returns the compile time  using __TIME__    in hex code 0xHHMM ver 0.1  returns always 0 
    unsigned short getCompileTime();
    // returns the compile date    using __DATE__     0xMMDD ver 0.1  returns always 0 
    unsigned short getCompileDate();
    // returns the compile year   using __DATE__  2014  0x140C ver 0.1  returns always 0 
    unsigned short getCompileYear();
    // takes a version nr of the type  "2.32"  and convert it to a hex nr   0x0220   ( 0x20== 33 dec) 
    void get_dec_version( unsigned short  hexversion , unsigned char & version, unsigned char& subversion);
    // translate  "2.32"  to 0x0222 
    unsigned short  get_hex_version_nr(const char * vers);
    // give a string with the HDR and SRC version 
    char* getversioninfo();
    
};    


#endif