#include "../Header/main.h"

//-------------------------------------- Defines, Enumerations ----------------------------------------------------------------
typedef enum
{
	SLOT_1,
	SLOT_2,
	SLOT_3,
	SLOT_4,
	SLOT_5,
    SLOT_WAIT,
	NUM_OF_MAIN_SLOTS
}MAIN_SLOT_TYPE;


//-------------------------------------- Global Variables ----------------------------------------------------------------

MAIN_SLOT_TYPE Slot;
MAIN_SLOT_TYPE prev_Slot;

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================


int main(void)
{

    Slot = SLOT_1;

    INITIALIZE_TASKS();

    int32_t prev_time = SysTick_GetTick(); // previous time

    while(1)
    {
        ALL_SLOTS_TASKS();


        switch(Slot){
			
			case SLOT_1:
				SLOT_1_TASKS();
			break;

			case SLOT_2:
				SLOT_2_TASKS();
			break;

			case SLOT_3:
				SLOT_3_TASKS();
			break;

			case SLOT_4:
				SLOT_4_TASKS();
			break;

			case SLOT_5:
				SLOT_5_TASKS();
			break;

            case SLOT_WAIT: // do nothing
            break;

			default:
                Slot = SLOT_1;
			break;

		}


        int32_t current_time = SysTick_GetTick();

        if (SysTick_GetElapsedTime(prev_time) >= SLOT_TIME) {
            
            prev_time = current_time;            
            Slot = prev_Slot;
            Slot++;
            if(Slot >= (SLOT_WAIT)){
                Slot = SLOT_1;
            }
        }
        else{
            if(Slot != SLOT_WAIT){
                prev_Slot = Slot;
            }

            Slot = SLOT_WAIT;
        }
       
    }
}