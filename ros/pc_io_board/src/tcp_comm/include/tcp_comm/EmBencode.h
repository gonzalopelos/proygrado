//
// Created by Gonzalo Pelós on 2/8/17.
//

#ifndef TCP_COMM_EMBENCODE_H
#define TCP_COMM_EMBENCODE_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/// Encoder class to generate Bencode on the fly (no buffer storage needed).
class EmBencode {
public:
    static void DebugPushData (const void* ptr, uint8_t len) {
        for (const char* p = (const char*) ptr; len-- > 0; ++p)
            PushChar(*p);
    }
    EmBencode () {}

    /// Push a string out in Bencode format.
    /// @param str The zero-terminated string to send out (without trailing \0).
    static void push (const char* str) {
        push(str, strlen(str));
    }
    /// Push arbitrary bytes in Bencode format.
    /// @param ptr Pointer to the data to send out.
    /// @param len Number of data bytes to send out.
    static void push (const void* ptr, uint8_t len) {
        PushCount(len);
        PushChar(':');
        PushData(ptr, len);
    }
    /// Push a signed integer in Bencode format.
    /// @param val The integer to send (this implementation supports 32 bits).
    static void push (long val) {
        PushChar('i');
        if (val < 0) {
            PushChar('-');
            val = -val;
        }
        PushCount(val);
        PushEnd();
    }

    static void startFrame () {
        lock(); // definido en mcc :(
        //serial_pkg.lock();
        //serial_pkg.wait();
        startList();
    }
    /// Start a new new list. Must be matched with a call to endList().
    /// Entries can be nested with more calls to startList(), startDict(), etc.
    static void startList () {
        PushChar('l');
    }
    /// Terminate a list, started earlier with a call to startList().
    static void endList () {
        PushEnd();
    }

    static void endFrame () {
        PushEnd();
        PushChar('\n');
        //serial_pkg.release();
        //serial_pkg.unlock();
        unlock(); // definido en mcc :(
    }

    /// Start a new new dictionary. Must be matched with a call to endDict().
    /// Dictionary entries must consist of a string key plus an arbitrary value.
    /// Entries can be nested with more calls to startList(), startDict(), etc.
    static void startDict () {
        PushChar('d');
    }
    /// Terminate a dictionary, started earlier with a call to startDict().
    static void endDict () {
        PushEnd();
    }

    static int append_data(const char * data){
        int result = -1;
        if((data[0] == 'l' || data[0] == 'd') && data[strlen(data) -1] == 'e'){
            PushData(&data[0u], strlen(data));
            result = strlen(data);
        }
        return result;
    }


protected:
    static void PushCount (uint32_t num) {
        char buf[11];
        //PushData(ultoa(num, buf, 10), strlen(buf));
        uint8_t len = snprintf(buf, 11, "%u", (unsigned int)num);
        PushData(buf, len);
    }
    static void PushEnd () {
        PushChar('e');
    }
    static void PushData (const void* ptr, uint8_t len) {
        for (const char* p = (const char*) ptr; len-- > 0; ++p)
            PushChar(*p);
    }
    /// This function is not implemented in the library. It must be supplied by
    /// the caller to implement the actual writing of characters.
    static void PushChar (char ch);

    static void lock ();
    static void unlock ();
};
/// Decoder class, needs an external buffer to collect the incoming data.
class EmBdecode {
public:
    /// Types of tokens, as returned by nextToken().
    enum { T_STRING = 0, T_NUMBER = 251, T_DICT, T_LIST, T_POP, T_END };
    /// Initialize a decoder instance with the specified buffer space.
    /// @param buf Pointer to the buffer which will be used by the decoder.
    /// @param len Size of the buffer, must be in the range 50 to 255.
    EmBdecode (char* buf, uint8_t len)
            : bufPtr (buf), bufLen (len) { reset(); }
    /// Reset the decoder - can be called to prepare for a new round of decoding.
    uint8_t reset ();
    /// Process a single incoming caharacter.
    /// @return Returns a count > 0 when the buffer contains a complete packet.
    uint8_t process (char ch);
    /// Call this after process() is done, to extract each of the data tokens.
    /// @returns Returns one of the T_STRING .. T_END enumeration codes.
    uint8_t nextToken ();
    /// Extract the last token as string (works for T_STRING and T_NUMBER).
    /// @param plen This variable will receive the size, if present.
    /// @return Returns pointer to a zero-terminated string in the decode buffer.
    char* asString (uint8_t* plen =0);
    /// Extract the last token as number (also works for strings if numeric).
    /// @return Returns the decoded integer, max 32-bit signed in this version.
    long asNumber ();

protected:
    void AddToBuf (char ch);
    char level, *bufPtr;
    uint8_t bufLen, count, next, last, state;
};
#endif //TCP_COMM_EMBENCODE_H
