#ifndef THRUST_MANAGER_H
#define THRUST_MANAGER_H

//------------------------------ Include Files ------------------------------------
#include "ch32v00x.h"
#include "SysError.h"
#include "ThrustManager_prm.h"

//----------------------------- Define, Enumerations -------------------------------
#pragma pack(push,1)
typedef struct __attribute__((packed)){ // Output Data
    int32_t current;
    int32_t voltage;
    int32_t power;
    int32_t Thrust;
    int32_t Torque;
    int16_t RPM;
    int16_t velocity; // 10 vezes a velocidae
    int16_t Duty;
    int8_t Err_table;
}Output_data;
#pragma pack(pop)

typedef struct{
    uint8_t Mode; // Calibration Mode, or Active Mode 
    uint32_t CalibrationFactorThrust;
    uint32_t CalibrationFactorTorque;
    uint16_t Duty;
}Input_data;

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