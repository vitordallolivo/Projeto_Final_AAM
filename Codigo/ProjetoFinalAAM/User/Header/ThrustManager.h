#ifndef THRUST_MANAGER_H
#define THRUST_MANAGER_H

//------------------------------ Include Files ------------------------------------
#include "ch32v00x.h"
#include "SysError.h"
#include "ThrustManager_prm.h"

//----------------------------- Define, Enumerations -------------------------------
#define SCALE_THRUST 1000      // 3 casas decimais
#define SCALE_TORQUE 1000
#define SCALE_VOLTAGE 1000     // milli-volts
#define SCALE_CURRENT 1000     // milli-amps
#define SCALE_POWER 1000       // milli-watts


//----------------------------- Public Functions (Prototypes)-----------------------
void CMPInitialize(void);
void CMP_BackgroundHandler(void); // Handles outputs/inputs
void CommunitacionHandle(void); // Handles with back and fourth communication

ERROR_TYPE SupervisionCMP(void); // check errors

// RPM handler
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void);
#endif