#ifndef PPM_H
#define PPM_H

// ----------------- Include Files ----------------
#include "ch32v00x.h"
#include "SysTick.h" // timer
//------------------ Defines, Enumerations ------------

//------------------ Functions (Prototypes)---------

void PPM_Initialize(void);
void PPM_Stop(void);
void PPM_SetValue(uint16_t value);


#endif