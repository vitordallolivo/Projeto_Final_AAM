#ifndef MAIN_PRM_H_
#define MAIN_PRM_H_


//-------------------------- Include Files ---------------------------------
#include "ch32v00x.h"
#include "Appl.h"
#include "SysTick.h"
#include "ADC.h"
#include "Hal.h"
#include "PWM.h"
#include "SysError.h"
#include "Sound.h"
#include "ThrustManager.h"
//-------------------------- Defines --------------------------------------


// Select ENALBED to use a fixed slot time duration for each main slot, otherwise select DISABLED
// If ENALBED, need to define the slot time duration in SLOT_TIME parameters below
#define USE_FIXED_TIME_SLOT	ENABLED //ENABLED//

// Define the fixed time slot duration
// Each slot in main loop will take SLOT_TIME before switch to next slot 
// Use _1MS for 1ms, _5MS for 5ms or _10MS for 10ms 

#define _1MS 10 // 1KHz
#define _2MS 20 // 500Hz
#define _4MS 40 // 250Hz
#define _10MS 100 // 100Hz
#define _1S 10000

#define SLOT_TIME	_1MS	//_1MS  // _4MS  // _10MS  



#define INITIALIZE_TASKS()\
{\
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);\
    SystemCoreClockUpdate();\
    SysTick_Init();\
    Sounds__Initialize();\
    ADC__Initialize();\
    Pwm__Initialize();\
    CMPInitialize();\
    Appl__Initialize();\
}


#define ALL_SLOTS_TASKS()\
{\
    Timer__MsHandler();\
	Hal__BackgroundHandler();\
    ADC_Handler();\
}

#define SLOT_1_TASKS()\
{\
    Appl__Handler();\
}

#define SLOT_2_TASKS()\
{\
    CMP_BackgroundHandler();\
}

#define SLOT_3_TASKS()\
{\
    Hal__BackgroundHandler();\
}

#define SLOT_4_TASKS()\
{\
    Error_Handler();\
}

#define SLOT_5_TASKS()\
{\
    Sounds__Background();\
}


#endif