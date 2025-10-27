#ifndef SYS_ERROR_H
#define SYS_ERROR_H

//----------------------------- Include Files -------------------------------------
#include "../Header/Hal.h"
//----------------------------- Defines, Enumerations -----------------------------

typedef enum {
    NONE = 0,
    NO_VOLTAGE,
    NO_CURRENT,
    MOTOR_NOT_SET,
    RC_RECEIVER_FAILED,        
    PITOT_NOT_ENGAGED,         
    RPM_NOT_ENGAGED,           
    LOAD_CELL_NOT_CONNECTED,   
    LOAD_CELL_NOT_CALIBRATED,
    MULTIPLE_ERRORS,
    NUM_ERRORS                
} ERROR_ID;

typedef enum {
    ERROR_MOTOR,
    ERROR_LOAD_CELL,
    ERROR_PITOT,
    NUM_ERROR_TYPES          
} ERROR_TYPE;

//------------------- Functions Prototypes ------------------------
void Error_Detect(ERROR_TYPE err, ERROR_ID id); // Fills the error vector
void Error_Handler(void);
uint8_t Error_GetStatus(ERROR_ID id);

#endif