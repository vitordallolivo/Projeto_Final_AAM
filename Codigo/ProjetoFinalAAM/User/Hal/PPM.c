//---------------------- Include Files ----------------
#include "../Header/PPM.h"
#include "../Header/PWM.h"
//------------------------- Defines, Enumerations --------------------------

typedef enum{
    PPM_IDLE,
    PPM_RUNNING
}PPM_state;

typedef enum {
    minimal,
    neutral,
    max,
    choose
} PPM_phase;

PPM_state Global_state; 
PPM_phase Global_phase;

#define PPM_FRAME_LENGTH_US    22500   // 22.5ms standard frame = 44.4Hz
#define PPM_SYNC_PULSE_US      4000    // Sync pulse longer than channel pulses
#define PPM_MIN_PULSE_US       1000    // 1ms minimum pulse
#define PPM_MAX_PULSE_US       2000    // 2ms maximum pulse  
#define PPM_GAP_US             300     // 300us gap between pulses

#define PPM_PWM_CHANNEL        PWM1    // Using PWM1 (PD3 - TIM2_CH2)

// Servo positions (aligned with PPM ranges)
#define SERVO_MIN_PULSE_WIDTH_US     PPM_MIN_PULSE_US      // 1000us = 0 degrees
#define SERVO_NEUTRAL_PULSE_WIDTH_US 1500                 // 1500us = 90 degrees  
#define SERVO_MAX_PULSE_WIDTH_US     PPM_MAX_PULSE_US      // 2000us = 180 degrees

#define MINIMAL_PWM_DUTY 197
#define MAX_PWM_DUTY 7537

//------------------------- Global Variables -------------------------------

u32 PPM_duty;
u8 Flag_PPM = 0;
u8 Timer_set = 0;

uint32_t time_period = 0;

//------------------------- Private Functions (Prototypes) -----------------


// =============================================================================
//                          FUNCOES PUBLICAS
// =============================================================================

void PPM_Initialize(void){
    uint32_t arr, psc;
    uint32_t timer_clock = SystemCoreClock; // 48MHz
    uint32_t frequency = 50;

    Pwm__SetTCFrequency(PWM_TIM2,frequency);
    Global_state = PPM_RUNNING;

    psc = (timer_clock / (frequency * 1000UL)) - 1;
    if(psc > 0xFFFF) psc = 0xFFFF;
    arr = (timer_clock / ((psc + 1) * frequency)) - 1;
    if(arr > 0xFFFF) arr = 0xFFFF;

    time_period = arr;


    PPM_SetValue(0);
}

void PPM_Stop(void){
    Global_state = PPM_IDLE;
    Pwm__SetDutyCycle(PWM1,3);
    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
}

void PPM_SetValue(uint16_t value){ // Pseudo-duty cycle  0% ->5% e 100% -> 10%
    
    if(Global_state == PPM_IDLE){
        value = 0;
    }
    
    // Constrain to valid range
    if(value > 100){
        value = 100;
    }

    PPM_duty = (time_period * (500 + 5 * value)) / 10000;
    TIM2->CH2CVR = PPM_duty;
    
}



// =============================================================================
//                          FUNCOES PRIVADAS
// =============================================================================
