// Embedded bencode support, header definitions.
// 2012-09-29 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include "EmBencode.h"


using namespace modules;

enum {
	EMB_ANY, EMB_LEN, EMB_INT, EMB_STR
};

/*************************************************************
 * 				EMBENCODE PUBLIC FUNCTIONS
 * ***********************************************************/

EmBencode::EmBencode() {

}

/// Push a string out in Bencode format.
/// @param str The zero-terminated string to send out (without trailing \0).
void EmBencode::push(const char* str) {
	push(str, strlen(str));
}

/// Push arbitrary bytes in Bencode format.
/// @param ptr Pointer to the data to send out.
/// @param len Number of data bytes to send out.
void EmBencode::push(const void* ptr, uint8_t len) {
	PushCount(len);
	PushChar(':');
	PushData(ptr, len);
}

/// Push a signed integer in Bencode format.
/// @param val The integer to send (this implementation supports 32 bits).
void EmBencode::push(long val) {
	PushChar('i');
	if (val < 0) {
		PushChar('-');
		val = -val;
	}
	PushCount(val);
	PushEnd();
}

void EmBencode::startFrame() {
	lock(); // definido en mcc :(
	//serial_pkg.lock();
	//serial_pkg.wait();
	startList();
}
/// Start a new new list. Must be matched with a call to endList().
/// Entries can be nested with more calls to startList(), startDict(), etc.
void EmBencode::startList() {
	PushChar('l');
}
/// Terminate a list, started earlier with a call to startList().
void EmBencode::endList() {
	PushEnd();
}

void EmBencode::endFrame() {
	PushEnd();
	PushChar('\n');
	//serial_pkg.release();
	//serial_pkg.unlock();
	unlock(); // definido en mcc :(
}

/// Start a new new dictionary. Must be matched with a call to endDict().
/// Dictionary entries must consist of a string key plus an arbitrary value.
/// Entries can be nested with more calls to startList(), startDict(), etc.
void EmBencode::startDict() {
	PushChar('d');
}
/// Terminate a dictionary, started earlier with a call to startDict().
void EmBencode::endDict() {
	PushEnd();
}

/*************************************************************
 * 				EMBENCODE :: PROTECTED FUNCTIONS
 * ***********************************************************/

void EmBencode::PushCount(uint32_t num) {
	char buf[11];
	//PushData(ultoa(num, buf, 10), strlen(buf));
	uint8_t len = snprintf(buf, 11, "%u", (unsigned int) num);
	PushData(buf, len);
}
void EmBencode::PushEnd() {
	PushChar('e');
}
void EmBencode::PushData(const void* ptr, uint8_t len) {
	for (const char* p = (const char*) ptr; len-- > 0; ++p)
		PushChar(*p);
}

/*************************************************************
 * 				EMBDECODE :: PUBLIC FUNCTIONS
 * ***********************************************************/

uint8_t EmBdecode::process(char ch) {
	switch (state) {
	case EMB_ANY:
		if (ch < '0' || ch > '9') {
			if (ch == 'i') {
				AddToBuf (T_NUMBER);
				state = EMB_INT;
			} else if (ch == 'd' || ch == 'l') {
				AddToBuf(ch == 'd' ? T_DICT : T_LIST);
				++level;
			} else if (ch == 'e') {
				AddToBuf (T_POP);
				--level;
				break; // end of dict or list
			}
			return 0;
		}
		state = EMB_LEN;
		count = 0;
		// fall through
	case EMB_LEN:
		if (ch == ':') {
			AddToBuf(T_STRING + count);
			if (count == 0) {
				AddToBuf(0);
				break; // empty string
			}
			state = EMB_STR;
		} else
			count = 10 * count + (ch - '0');
		return 0;
	case EMB_STR:
		AddToBuf(ch);
		if (--count == 0) {
			AddToBuf(0);
			break; // end of string
		}
		return 0;
	case EMB_INT:
		if (ch == 'e') {
			AddToBuf(0);
			break; // end of int
		}
		AddToBuf(ch);
		return 0;
	}
	// end of an item reached
	if (level > 0) {
		state = EMB_ANY;
		return 0;
	}
	AddToBuf (T_END);
	return reset(); // not in dict or list, data is complete
}

uint8_t EmBdecode::nextToken() {
	uint8_t ch = bufPtr[next++];
	last = next;
	switch (ch) {
	default: // string
		next += ch + 1;
		return T_STRING;
	case T_NUMBER:
		while (bufPtr[next++] != 0)
			;
		break;
	case T_END:
		--next; // don't advance past end token
		// fall through
	case T_DICT:
	case T_LIST:
	case T_POP:
		break;
	}
	return ch;
}

char* EmBdecode::asString(uint8_t* plen) {
	if (plen != 0)
		*plen = next - last - 1;
	return bufPtr + last;
}

long EmBdecode::asNumber() {
	return atol(bufPtr + last);
}

uint8_t EmBdecode::reset() {
	count = next;
	level = next = 0;
	state = EMB_ANY;
	return count;
}

/*************************************************************
 * 				EMBDECODE :: PROTECTED FUNCTIONS
 * ***********************************************************/

void EmBdecode::AddToBuf(char ch) {
	if (next >= bufLen)
		bufPtr[0] = T_END; // mark entire buffer as empty
	else
		bufPtr[next++] = ch;
}
