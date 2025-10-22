//------------------------ Include Files -----------------------
#include "../Header/Sound.h"
#include "../Header/Hal.h"
#include "../Header/SysTick.h"

//------------------------- Defines, Enumerations --------------------------
typedef enum{
    SOUND_DONE=0,
    SOUNDS_PLAYING=1
}SOUND_STATUS;
//------------------------- Global Variables -------------------------------
SOUNDS_TYPE PlaySound;
ERROR_ID id_Sound;
uint32_t on_tick = 0UL;
uint32_t off_tick = 0UL;
//------------------------- Private Functions (Prototypes) -----------------
SOUND_STATUS ErrorSoundHandler(void);

// =============================================================================
//                          FUNCOES PUBLICAS
// =============================================================================

void Sounds__Initialize(void){
    PlaySound = PLAY_NO_SOUND;
}
void Sounds__Background(void){
    switch (PlaySound) {

        case PLAY_ERROR:

            if(ErrorSoundHandler() == SOUND_DONE){
                PlaySound = PLAY_NO_SOUND;
            }

            break;
        case PLAY_NO_SOUND:
            Hal__SetBuzzer(0);
            break;
        default:
            break;
    }
}
void Sounds__PlaySounds( SOUNDS_TYPE sound_id, ERROR_ID error_id){
    if(sound_id<NUM_OF_SOUNDS){
        PlaySound = sound_id;
        id_Sound = error_id;
    }
}

// =============================================================================
//                          FUNCOES PRIVADAS
// =============================================================================


SOUND_STATUS ErrorSoundHandler(void){
    if(id_Sound !=NONE){ // Decrementa o id_sound sempre

        static unsigned char state = 0xFF;
        static unsigned char state_buzzer = 1;

        if(SysTick_GetElapsedTime(on_tick) >= 250){ // 250 milisegundos
            
            if(state == 0xFF){
                state = 2*id_Sound;
            }
            else{
                state--;
            }

            if(state == 0){
                id_Sound = NONE;
                state = 0xFF;
            }
            state_buzzer = !state_buzzer; 
            Hal__SetBuzzer(state_buzzer);
            on_tick = SysTick_GetTick(); 
        }        

        return SOUNDS_PLAYING;
    }
    else{
        return SOUND_DONE;
    }
}