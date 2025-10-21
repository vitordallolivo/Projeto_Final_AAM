#ifndef PWM_PRM_H_
#define PWM_PRM_H_

#include "ch32v00x.h"

#define DISABLED    0
#define ENABLED     1

// Configuracao dos canais PWM - APENAS PINOS EXISTENTES NO CH32V003F4P6
#define CONFIG_PWM0     DISABLED     // PD2 - TIM2_CH3
#define CONFIG_PWM1     ENABLED     // PD3 - TIM2_CH2  
#define CONFIG_PWM2     DISABLED     // PD4 - TIM2_CH1
#define CONFIG_PWM3     DISABLED     // PD5 - TIM2_CH4
#define CONFIG_PWM4     DISABLED     // PC0 - TIM2_CH3
#define CONFIG_PWM5     DISABLED     // PC1 - TIM2_CH4

// Frequ¨ºncia padrao dos timers (Hz)
#define PWM_TIM1_FREQUENCY     1000
#define PWM_TIM2_FREQUENCY     60

#define PWM_TIM1_SET DISABLED
#define PWM_TIM2_SET ENABLED

#endif /* PWM_PRM_H_ */