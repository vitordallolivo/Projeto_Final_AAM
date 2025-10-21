#include "ch32v00x.h"
#include "../Header/PWM.h"

//-------------------------------------- Defines, Enumerations ----------------------------------------------------------------

const unsigned char PWM_Enable_Table[NUM_OF_PWM] = {
    CONFIG_PWM0, // PD2 - TIM2_CH3
    CONFIG_PWM1, // PD3 - TIM2_CH2  
    CONFIG_PWM2, // PD4 - TIM2_CH1
    CONFIG_PWM3, // PD5 - TIM2_CH4
    CONFIG_PWM4, // PC0 - TIM2_CH3
    CONFIG_PWM5  // PC1 - TIM2_CH4
};

//-------------------------------------- Global Variables  --------------------------------------------------------------------

static uint16_t timer_period[NUM_OF_PWM_TIM] = {0};

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

void Pwm__Initialize(void)
{
    PWM_ID_TYPE PWM_id;
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    
    // Configura apenas os PWMs habilitados
    for(PWM_id = PWM0; PWM_id < NUM_OF_PWM; PWM_id++)
    {
        if(PWM_Enable_Table[PWM_id] == ENABLED)
        { 
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            
            switch(PWM_id)
            {

                #if (CONFIG_PWM0==ENABLED)
                    case PWM0: // PD2 - TIM2_CH3
                        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
                        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
                        GPIO_Init(GPIOD, &GPIO_InitStructure);
                        break;
                #endif

                #if (CONFIG_PWM1==ENABLED)
                    case PWM1: // PD3 - TIM2_CH2
                        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
                        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
                        GPIO_Init(GPIOD, &GPIO_InitStructure);
                        break;
                
                #endif

                #if (CONFIG_PWM2==ENABLED)
                case PWM2: // PD4 - TIM2_CH1
                    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                    GPIO_Init(GPIOD, &GPIO_InitStructure);
                    break;

                #endif
                #if (CONFIG_PWM3==ENABLED)
                case PWM3: // PD5 - TIM2_CH4
                    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
                    GPIO_Init(GPIOD, &GPIO_InitStructure);
                    break;
                #endif
                #if (CONFIG_PWM4==ENABLED)
                case PWM4: // PC0 - TIM2_CH3
                    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
                    GPIO_Init(GPIOC, &GPIO_InitStructure);
                    break;

                #endif
                #if (CONFIG_PWM5==ENABLED)
                case PWM5: // PC1 - TIM2_CH4
                    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
                    GPIO_Init(GPIOC, &GPIO_InitStructure);
                    break;
                #endif
                default:
                    break;
            }
            
            // Configura duty cycle inicial 0%
            Pwm__SetTCFrequency(PWM_TIM2,60);
            Pwm__SetDutyCycle(PWM_id, 10);
        }
    }
    
    TIM_Cmd(TIM2, ENABLE);
}

void Pwm__SetTCFrequency(PWM_TIM_TYPE tc, unsigned short frequency)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    uint32_t timer_clock = SystemCoreClock; // 48MHz
    
    // FORMULA CORRETA:
    // Frequency = Timer_Clock / ((PSC + 1) * (ARR + 1))
    // Para 60Hz com timer de 1MHz:
    // 60Hz = 48,000,000 / ((PSC + 1) * (ARR + 1))
    
    uint32_t arr, psc;
    
    if(frequency == 60) {
        // Configuracao especifica para 60Hz (servos/ESC)
        psc = 799;      // 48,000,000 / 800 = 60,000Hz (60KHz)
        arr = 999;      // 60,000 / 1000 = 60Hz
    }
    else {
        // C¨¢lculo generico
        psc = (timer_clock / (frequency * 1000UL)) - 1;
        if(psc > 0xFFFF) psc = 0xFFFF;
        
        arr = (timer_clock / ((psc + 1) * frequency)) - 1;
        if(arr > 0xFFFF) arr = 0xFFFF;
    }
    
    timer_period[tc] = arr;
    
    switch(tc)
    {

        #if (PWM_TIM1_SET==ENABLED)
            case PWM_TIM1:
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
                TIM_TimeBaseInitStructure.TIM_Period = arr;
                TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
                TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
                TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
                TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
                TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
                TIM_CtrlPWMOutputs(TIM1, ENABLE);
                TIM_Cmd(TIM1, ENABLE);
                break;
        #endif

        #if (PWM_TIM2_SET==ENABLED)
            case PWM_TIM2:
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
                TIM_TimeBaseInitStructure.TIM_Period = arr;
                TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
                TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
                TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
                TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
                TIM_Cmd(TIM2, ENABLE);
                break;
        #endif

        default:
            break;
    }
}

void Pwm__SetDutyCycle(PWM_ID_TYPE pwm, unsigned char duty)
{
    static TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    uint16_t pulse;
    
    if(pwm>NUM_OF_PWM){
        return;
    }

    if(duty > 100)
    { 
        duty = 100;
    }
    
    // Calcula pulse width baseado no per¨ªodo do timer
    pulse = (timer_period[PWM_TIM2] * duty) / 100;
    
    // Configura estrutura do fabricante
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = pulse;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    switch(pwm)
    {
        #if (CONFIG_PWM0==ENABLED)
            case PWM0: // TIM2_CH3
                TIM_OC3Init(TIM2, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
                break;
        #endif
        #if (CONFIG_PWM1==ENABLED)
            case PWM1: // TIM2_CH2
                TIM_OC2Init(TIM2, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
                break;
        #endif
        #if (CONFIG_PWM2==ENABLED)
            case PWM2: // TIM2_CH1
                TIM_OC1Init(TIM2, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
                break;
        #endif
        #if (CONFIG_PWM3==ENABLED)
            case PWM3: // TIM2_CH4
                TIM_OC4Init(TIM2, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
                break;
        #endif
        #if (CONFIG_PWM4==ENABLED)
            case PWM4: // TIM2_CH3 (PC0)
                TIM_OC3Init(TIM2, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
                break;
        #endif
        #if (CONFIG_PWM5==ENABLED)
            case PWM5: // TIM2_CH4 (PC1)
                TIM_OC4Init(TIM2, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
                break;
        #endif  
        default:
            break;
    }
    
    TIM_ARRPreloadConfig(TIM2, ENABLE);
}