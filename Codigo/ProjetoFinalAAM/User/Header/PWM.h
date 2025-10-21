#ifndef PWM_H_
#define PWM_H_
//--------------------- Included Files -------------------
#include "ch32v00x.h"
#include "PWM_prm.h"
// -------------------- Defines, Enumerations ---------------

typedef enum {
    PWM0 = 0,   // PD2 - TIM2_CH3
    PWM1,       // PD3 - TIM2_CH2
    PWM2,       // PD4 - TIM2_CH1
    PWM3,       // PD5 - TIM2_CH4
    PWM4,       // PC0 - TIM2_CH3
    PWM5,       // PC1 - TIM2_CH4
    NUM_OF_PWM,
} PWM_ID_TYPE;

typedef enum {
    PWM_TIM1 = 0,   // Advanced-control timer (N?O FOI IMPLEMENTADO)
    PWM_TIM2,       // General-purpose timer
    NUM_OF_PWM_TIM,
} PWM_TIM_TYPE;

// -------------------- Function's Prototypes ---------------

// Prot¨®tipos
void Pwm__Initialize(void);
void Pwm__SetTCFrequency(PWM_TIM_TYPE timer, uint16_t frequency);
void Pwm__SetDutyCycle(PWM_ID_TYPE pwm, uint8_t duty);
;

#endif /* PWM_H_ */