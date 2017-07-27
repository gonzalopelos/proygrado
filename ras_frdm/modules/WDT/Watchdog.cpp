#include "mbed.h"
#include "Watchdog.h"
#include "fsl_rcm.h"

using namespace modules;

#if defined(TARGET_K64F)

Watchdog::Watchdog(){
	reportResetStatus();
	const uint32_t K64FWDONESEC = 0x00B80000;   //approx 1 second delay
	if((toVal < 1) || (toVal > 356)) toVal = 356;
	WDOG->UNLOCK = 0xC520;
	WDOG->UNLOCK = 0xD928;
	uint32_t wdogCntl = K64FWDONESEC * toVal;
	WDOG->WINH = 0;
	WDOG->WINL = 0;
	WDOG->TOVALH = wdogCntl >> 16;
	WDOG->TOVALL = wdogCntl & 0xFFFF;
	WDOG->STCTRLH |= 0x0001;
	//EnableWDOG();
	kick();
}

void Watchdog::kick() {
	/* Se deben deshabilitar las interrupciones para realizar el refresh del WDT debido a que entre
	 * la escritura de la primer palabra y la segunda deben transcurrir como maximo 20 ciclos del bus clock.
	 * Las interrupciones se pueden "colar" entre la primer y segunda asginacion pudiendo no cumplirse la restriccion.
	 */
	__disable_irq();
	WDOG->REFRESH = 0xA602;
	WDOG->REFRESH = 0xB480;
	__enable_irq();
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

int Watchdog::getTimeOutValue(){
	return toVal;
}

void Watchdog::reportResetStatus(){
	uint32_t resetStatus = RCM_GetPreviousResetSources(RCM) & (kRCM_SourceWdog | kRCM_SourcePin | kRCM_SourcePor);
	switch(resetStatus){
		case kRCM_SourceWdog:
			printf("WATCHDOG TIMEOUT RESET\r\n");
			break;
		case kRCM_SourcePin:
			printf("EXTERNAL PIN RESET\r\n");
			break;
		case kRCM_SourcePor:
			printf("POWER ON RESET\r\n");
			break;
		default:
			printf("OTHER SOURCE RESET\r\n");
			break;
	}
}

#endif
