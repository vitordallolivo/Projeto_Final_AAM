//------------------------------- Include Files ---------------------------
#include "../Header/Hal.h"
#include "../Header/SysError.h"
#include "../Header/SysTick.h"
#include "../Header/Sound.h"
//------------------------------- Defines, Enumerations -----------------

#define DETECT 1
#define NONE_DETECT 0 

#define MESSAGE_ERROR_ID 0
#define MESSAGE_ERROR_TYPE 1

#define ERROR_PLAY_INTERVAL 10000  

// ------------------------------- Global Variables ------------------------------- 

ERROR_ID err_vect[NUM_ERRORS]={0};

uint8_t  err_last = 0;
uint32_t tick_error = 0UL;

//----------------------------- Private Functions -----------------------------

// =============================================================================
//                          FUNCOES PUBLICAS
// =============================================================================


void Error_Detect(ERROR_TYPE err,ERROR_ID id){
    if(id != NONE){
        err_vect[id] = DETECT; // ERRO DETECTADO
    }
}
void Error_Handler(void){

    uint8_t  error_to_play[NUM_ERRORS] = {0};
    unsigned char DetectErrorNum = 0; // numero de erros detectados


    for(int id=0;id<NUM_ERRORS;id++){ // andar por todos os erros
        if(err_vect[id] == DETECT){

            error_to_play[DetectErrorNum] = id;
            DetectErrorNum++;
        }
    }

    if(DetectErrorNum ==0){
        return;
    }
    

    if(SysTick_GetElapsedTime(tick_error) > ERROR_PLAY_INTERVAL){

        if(DetectErrorNum == 1){
            Sounds__PlaySounds(PLAY_ERROR,error_to_play[0]);
        }
        else{
            
            Sounds__PlaySounds(PLAY_ERROR,error_to_play[err_last]);

            if(err_last == DetectErrorNum){
                err_last = 0;
            }else{
                err_last++;
            }
        }
        tick_error = SysTick_GetTick();

    }

}
uint8_t Error_GetStatus(ERROR_ID id){
    if(err_vect[id] == DETECT){
        return true;
    }
    return false;
}

// =============================================================================
//                          FUNCOES PRIVADA
// =============================================================================
