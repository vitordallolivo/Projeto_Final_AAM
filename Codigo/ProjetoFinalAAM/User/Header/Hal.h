#ifndef HAL_H
#define HAL_H

//---------------------- Include Files -----------------------------
#include "ch32v00x.h"
#include "Hal_prm.h"
//---------------------  Defines, Enumerations----------------------

typedef enum{
    Current_pin = 0,
    Voltage_pin,
    NUM_OF_ANALOG_INPUT
}ANALOG_INPUT_TYPE;

//=====================================================================================================================
//--------------------------------------Funcoes Publicas (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void Hal__Initialize(void);
void Hal__BackgroundHandler(void);
void Hal__FastHandler(void);

// ADC
unsigned short int Hal_GetAnalogInput(ANALOG_INPUT_TYPE input);


#endif