#ifndef WATCHDOG_K64F_H
#define WATCHDOG_K64F_H

#include "mbed.h"

namespace modules{

class Watchdog {

private:
	int toVal = 5;	// Timeout Value - Maximo 356 segundos, un valor mayor desborda los regsitros TOVALH y TOVALL del Watchdog.

    // Desabilitar WDT
    void DisableWDOG();
    
    // Habilitar WDT
    void EnableWDOG();

    // Reportar la fuente del ultimo reset (POWER ON, EXTERNAL PIN, WATCHDOG, OTRO).
    void reportResetStatus();

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
};

}

#endif
