#ifndef THRUST_MANAGER_H
#define THRUST_MANAGER_H

//------------------------------ Include Files ------------------------------------
#include "ch32v00x.h"
#include "SysError.h"
#include "ThrustManager_prm.h"

//----------------------------- Define, Enumerations -------------------------------

typedef enum{
    Configuration,
    Released_Action,
    NO_EVENT
}UserAction;

typedef enum{
    Released,
    Turn_off
}MotorAction;

//----------------------------- Public Functions (Prototypes)-----------------------
void CMPInitialize(void);
void CMP_BackgroundHandler(void); // Handles outputs/inputs

UserAction SupervisionCMP(void); // check errors
void CheckCriticalError(void);


void ThrustManager_SetMotorAction(MotorAction act);
void ThrustManager_SetLoadCell(void);

void Thrust_UpdateData(void);

#endif