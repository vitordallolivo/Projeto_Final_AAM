#ifndef HAL_H
#define HAL_H

//---------------------- Include Files -----------------------------
#include "ch32v00x.h"
#include "Hal_prm.h"
//---------------------  Defines, Enumerations----------------------

typedef enum{
    Current_pin = 2,
    Voltage_pin,
    NUM_OF_ANALOG_INPUT
}ANALOG_INPUT_TYPE;

typedef enum{
    Thrust_Cell,
    Torque_Cell,
    NUM_OF_LOAD_CELL
}LOAD_CELL_TYPE;

//=====================================================================================================================
//--------------------------------------Funcoes Publicas (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void Hal__Initialize(void);
void Hal__BackgroundHandler(void);
void Hal__FastHandler(void);

// ADC
unsigned short int Hal_GetAnalogInput(ANALOG_INPUT_TYPE input);

// HX711
void LoadCellSelect(LOAD_CELL_TYPE cell);
void LoadCellRead(void);
uint32_t GetCellRead(LOAD_CELL_TYPE cell);

// Motor
void Hal_SetMotor(unsigned char state);
void Hal_SetMotorDuty(unsigned char duty);


// Buzzer
void Hal__SetBuzzer(unsigned char state);
void Hal_SetBuzzerDutyCycle(unsigned char duty);
void Hal__SetBuzzerFreq(unsigned short int frequency);


#endif