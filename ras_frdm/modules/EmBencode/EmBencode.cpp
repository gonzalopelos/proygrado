// Embedded bencode support, header definitions.
// 2012-09-29 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include "EmBencode.h"

enum {
	EMB_ANY, EMB_LEN, EMB_INT, EMB_STR
};


uint8_t EmBdecode::reset() {
	count = next;
	level = next = 0;
	state = EMB_ANY;
	return count;
}

uint8_t EmBdecode::process(char ch) {
	switch (state) {
	case EMB_ANY:
		if (ch < '0' || ch > '9') {
			if (ch == 'i') {
				AddToBuf(T_NUMBER);
				state = EMB_INT;
			} else if (ch == 'd' || ch == 'l') {
				AddToBuf(ch == 'd' ? T_DICT : T_LIST);
				++level;
			} else if (ch == 'e') {
				AddToBuf(T_POP);
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
	AddToBuf(T_END);
	return reset(); // not in dict or list, data is complete
}

void EmBdecode::AddToBuf(char ch) {
	if (next >= bufLen)
		bufPtr[0] = T_END; // mark entire buffer as empty
	else
		bufPtr[next++] = ch;
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
