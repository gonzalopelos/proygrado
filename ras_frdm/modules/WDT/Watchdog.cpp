#include "mbed.h"
#include "Watchdog.h"

using namespace modules;

#if defined(TARGET_K64F)

void Watchdog::kick(int toVal) {
    const uint32_t K64FWDONESEC = 0x00B80000;   //approx 1 second delay
    if((toVal < 1) || (toVal > 356)) toVal = 356;
    WDOG->UNLOCK = 0xC520;
    WDOG->UNLOCK = 0xD928;
    uint32_t wdogCntl = K64FWDONESEC * toVal;
    WDOG->TOVALH = wdogCntl >> 16;
    WDOG->TOVALL = wdogCntl & 0xFFFF;
    EnableWDOG();
}

void Watchdog::kick() {
	WDOG->REFRESH = 0xA602;
	WDOG->REFRESH = 0xB480;
}

void Watchdog::DisableWDOG() {   
	WDOG->UNLOCK = 0xC520;
	WDOG->UNLOCK = 0xD928;
	WDOG->STCTRLH &= 0xFFFE;
}
    
void Watchdog::EnableWDOG() {   
	WDOG->UNLOCK = 0xC520;
	WDOG->UNLOCK = 0xD928;
	WDOG->STCTRLH |= 0x0001;
    kick();
}

#endif
