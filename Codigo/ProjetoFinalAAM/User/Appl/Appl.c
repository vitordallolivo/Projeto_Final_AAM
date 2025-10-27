//---------------------------------------- Include Files ---------------------------------
#include "../Header/ThrustManager.h"
#include "../Header/SysError.h"
#include "../Header/SysTick.h"
#include "../Header/Hal.h"
#include "../Header/Sound.h"
//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------

//-------------------------------------- Defines, Enumerations ----------------------------------------------------------------



//-------------------------------------- Global Variables ----------------------------------------------------------------
UserAction Status;
//-------------------------------------- PRIVATE (Function Prototypes) ---------------------------------------------------


//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================


void Appl__Initialize(void){
    Status = Configuration;
}

void Appl__Handler(void){

    Status = SupervisionCMP();

    if(Status !=NO_EVENT){
        switch (Status) {
            case Configuration:
                ThrustManager_SetMotorAction(Turn_off);
                ThrustManager_SetLoadCell();
            break;
            case Released_Action:
                ThrustManager_SetMotorAction(Released);
            break;

            default:
            break;
        }
    }

    CheckCriticalError();

}