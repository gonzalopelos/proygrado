/// @file
/// Embedded bencode support, header definitions.
// 2012-09-29 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
#ifndef __EmBencode_h
#define __EmBencode_h
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


namespace modules {
/// Encoder class to generate Bencode on the fly (no buffer storage needed).
class EmBencode {
public:

	EmBencode();

	/// Push a string out in Bencode format.
	/// @param str The zero-terminated string to send out (without trailing \0).
	void push(const char* str);

	/// Push arbitrary bytes in Bencode format.
	/// @param ptr Pointer to the data to send out.
	/// @param len Number of data bytes to send out.
	static void push(const void* ptr, uint8_t len);

	/// Push a signed integer in Bencode format.
	/// @param val The integer to send (this implementation supports 32 bits).
	static void push(long val);

	static void startFrame();
	/// Start a new new list. Must be matched with a call to endList().
	/// Entries can be nested with more calls to startList(), startDict(), etc.
	static void startList();
	/// Terminate a list, started earlier with a call to startList().
	static void endList();

	static void endFrame();

	/// Start a new new dictionary. Must be matched with a call to endDict().
	/// Dictionary entries must consist of a string key plus an arbitrary value.
	/// Entries can be nested with more calls to startList(), startDict(), etc.
	static void startDict();
	/// Terminate a dictionary, started earlier with a call to startDict().
	static void endDict();
protected:
	static void PushCount(uint32_t num);
	static void PushEnd();
	static void PushData(const void* ptr, uint8_t len);
	/// This function is not implemented in the library. It must be supplied by
	/// the caller to implement the actual writing of characters.
	static void PushChar(char ch);

	static void lock();
	static void unlock();
};
/// Decoder class, needs an external buffer to collect the incoming data.
class EmBdecode {
public:
	/// Types of tokens, as returned by nextToken().
	enum {
		T_STRING = 0, T_NUMBER = 251, T_DICT, T_LIST, T_POP, T_END
	};
	/// Initialize a decoder instance with the specified buffer space.
	/// @param buf Pointer to the buffer which will be used by the decoder.
	/// @param len Size of the buffer, must be in the range 50 to 255.
	EmBdecode(char* buf, uint8_t len) :
			bufPtr(buf), bufLen(len) {
		reset();
	}
	/// Reset the decoder - can be called to prepare for a new round of decoding.
	uint8_t reset();
	/// Process a single incoming caharacter.
	/// @return Returns a count > 0 when the buffer contains a complete packet.
	uint8_t process(char ch);
	/// Call this after process() is done, to extract each of the data tokens.
	/// @returns Returns one of the T_STRING .. T_END enumeration codes.
	uint8_t nextToken();
	/// Extract the last token as string (works for T_STRING and T_NUMBER).
	/// @param plen This variable will receive the size, if present.
	/// @return Returns pointer to a zero-terminated string in the decode buffer.
	char* asString(uint8_t* plen = 0);
	/// Extract the last token as number (also works for strings if numeric).
	/// @return Returns the decoded integer, max 32-bit signed in this version.
	long asNumber();

protected:
	void AddToBuf(char ch);
	char level, *bufPtr;
	uint8_t bufLen, count, next, last, state;
};
}
#endif
