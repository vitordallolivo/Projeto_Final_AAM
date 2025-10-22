#ifndef MAIN_PRM_H_
#define MAIN_PRM_H_


//-------------------------- Include Files ---------------------------------
#include "ch32v00x.h"
#include "SysTick.h"
#include "ADC.h"
#include "Hal.h"
#include "PWM.h"
#include "SysError.h"
#include "Sound.h"
//-------------------------- Defines --------------------------------------


// Select ENALBED to use a fixed slot time duration for each main slot, otherwise select DISABLED
// If ENALBED, need to define the slot time duration in SLOT_TIME parameters below
#define USE_FIXED_TIME_SLOT	ENABLED //ENABLED//

// Define the fixed time slot duration
// Each slot in main loop will take SLOT_TIME before switch to next slot 
// Use _1MS for 1ms, _5MS for 5ms or _10MS for 10ms 

#define _1MS 1 // 1KHz
#define _2MS 2 // 500Hz
#define _4MS 4 // 250Hz
#define _10MS 10 // 100Hz
#define _1S 1000



#define SLOT_TIME	_1MS	//_1MS  // _4MS  // _10MS  



#define INITIALIZE_TASKS()\
{\
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);\
    SystemCoreClockUpdate();\
    USART_Printf_Init(115200);\
    SysTick_Init();\
    Sounds__Initialize();\
    ADC__Initialize();\
    Pwm__Initialize();\
}


#define ALL_SLOTS_TASKS()\
{\
	Hal__BackgroundHandler();\
    ADC_Handler();\
}

#define SLOT_1_TASKS()\
{\
}

#define SLOT_2_TASKS()\
{\
}

#define SLOT_3_TASKS()\
{\
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