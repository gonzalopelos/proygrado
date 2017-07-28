#ifndef WATCHDOG_K64F_H
#define WATCHDOG_K64F_H

#include "mbed.h"

namespace modules{

class Watchdog {

private:
	int toVal = 5;	// Timeout Value - Maximo 356 segundos, un valor mayor desborda los regsitros TOVALH y TOVALL del Watchdog.
	uint32_t resetStatus;

    // Desabilitar WDT
    void DisableWDOG();
    
    // Habilitar WDT
    void EnableWDOG();

public:
    // ***************************************************************************************************
	// * Constructor
	// *
	// * Este metodo debe ser la primer o una de las primeras cosas que se debe invocar luego de un reset.
	// ***************************************************************************************************
	Watchdog();

    // Patear el perro.
    void kick();

    // Obtener valor de timeout del Watchdog.
    int getTimeOutValue();

    // Obtener fuente del ultimo reset (POWER ON, EXTERNAL PIN, WATCHDOG)
    uint32_t getLastResetStatus();
};

}

#endif
