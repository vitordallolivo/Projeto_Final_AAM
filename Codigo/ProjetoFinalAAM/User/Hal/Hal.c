//------------------------ Include Files ----------------------------------
#include "ch32v00x_gpio.h"
#include "../Header/Hal.h"
#include "../Header/ADC.h"
#include "../Header/PWM.h"

//------------------------ Global Variables ----------------------------------
// Variaveis para ADC
AD_CHANNEL_TYPE AD_Channel;
unsigned short int AD_Accumulator[NUM_OF_ANALOG_INPUT];
unsigned short int AD_Sample[NUM_OF_ANALOG_INPUT];
unsigned char AD_Counter[NUM_OF_ANALOG_INPUT];

unsigned short int Hal_Analog_Inputs[NUM_OF_ANALOG_INPUT];   // buffer da m¨¦dia do valor do ADC de cada canal

uint32_t LoadCellStore[NUM_OF_LOAD_CELL] = {0};
uint32_t LoadCellOffset[NUM_OF_LOAD_CELL] = {0};

LOAD_CELL_TYPE SelectCell = NUM_OF_LOAD_CELL;


//------------------------ Defines, Enumerations ----------------------------------


#define NUM_AD_SAMPLES        8

#define GPIO_HX711_DT  GPIO_Pin_1 // PC1 
#define GPIO_HX711_SCK  GPIO_Pin_2 // PC0

#define NUM_BYTES_LOAD_CELL 24


//------------------------ Private Functions (prototypes) ----------------------------------
void ADProcess(void);


//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

void Hal__Initialize(void){
    
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Inicializando celula de carga

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLED);
    GPIO_InitStructure.GPIO_Mode = GPIO_HX711_DT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_HX711_SCK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    // Inicializando interrup??es por pino
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
    
    // 2. Configurar PD0 como entrada pull-up
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // 3. Configurar PD2 como entrada pull-up
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // 4. Configurar PD0 para EXTI0 (borda de subida)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
    
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    // 5. Configurar PD2 para EXTI2 (borda de subida e descida)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2);
    
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    // 6. Configurar NVIC para EXTI0
    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void Hal__BackgroundHandler(void){
    ADProcess();
    LoadCellRead();
}
void Hal__FastHandler(void){

}

//ADC

unsigned short int Hal_GetAnalogInput(ANALOG_INPUT_TYPE input){
    return Hal_Analog_Inputs[input];
}

// LOAD_CELL

void LoadCellSelect(LOAD_CELL_TYPE cell){
    SelectCell = cell;
}
void LoadCellRead(void){
    u_int32_t valor = 0;
    uint8_t i =0;
    
    if(!GPIO_ReadInputDataBit(GPIOC,GPIO_HX711_DT)){
        return;
    }

    for(i = 0;i < NUM_BYTES_LOAD_CELL; i++){
        GPIO_WriteBit(GPIOC,GPIO_HX711_SCK,Bit_SET);

        valor<<=1;
        if(GPIO_ReadInputDataBit(GPIOC, GPIO_HX711_DT)){
            valor++;
        }
        
        GPIO_WriteBit(GPIOC, GPIO_HX711_SCK, Bit_RESET);
    }

    uint8_t pulsos = (SelectCell == Thrust_Cell)? 1 : 2;

    for(i = 0; i < pulsos; i++){
        GPIO_WriteBit(GPIOC, GPIO_HX711_SCK, Bit_SET);
        GPIO_WriteBit(GPIOC, GPIO_HX711_SCK, Bit_RESET);
    }

    if(valor & 0x00800000){
        valor |= 0xFF000000;
    }

    // Armazena no vetor
    LoadCellStore[SelectCell] = valor;
}


uint32_t GetCellRead(LOAD_CELL_TYPE cell){
    return LoadCellStore[cell];
}

void LoadCellTare(LOAD_CELL_TYPE cell){
    LoadCellOffset[cell] = LoadCellStore[cell];
}




// Motor
void Hal_SetMotor(unsigned char state){
    if(state > 0){
        Pwm__SetDutyCycle(PWM1,20);
    }
    else{
            Pwm__SetDutyCycle(PWM1,0);
    }
}
void Hal_SetMotorDuty(unsigned char duty){
    Pwm__SetDutyCycle(PWM1,duty);
}




// Buzzer
void Hal__SetBuzzer(unsigned char state){
    #if (USED_BUZZER == ENABLE)

        if(state > 0){
            Pwm__SetDutyCycle(PWM6,5);
        }
        else{
            Pwm__SetDutyCycle(PWM6,0);
        }

    #endif
}
void Hal_SetBuzzerDutyCycle(unsigned char duty){
    Pwm__SetDutyCycle(PWM6,duty);
}
void Hal__SetBuzzerFreq(unsigned short int frequency){
    Pwm__SetTCFrequency(PWM_TIM2,frequency);
}


//=====================================================================================================================
//-------------------------------------- Private Functions -------------------------------------------------------------
//=====================================================================================================================
void ADProcess(void)
{
    if(Adc_Enable_Table[AD_Channel] == SET)
    {
        if(AD_Counter[AD_Channel] < NUM_AD_SAMPLES)
        {
            AD_Sample[AD_Channel] = ADC__GetValue(AD_Channel);
            AD_Accumulator[AD_Channel] += AD_Sample[AD_Channel];
            AD_Counter[AD_Channel]++;
        }
        else
        {  // O Buffer Hal_Analog_Inputs[] ¨¦ a m¨¦dia do valor de ADC
            Hal_Analog_Inputs[AD_Channel] = (unsigned short int)(AD_Accumulator[AD_Channel]/NUM_AD_SAMPLES);
            AD_Accumulator[AD_Channel] = 0;
            AD_Counter[AD_Channel] = 0;
        }
    }
    AD_Channel++;
    if(AD_Channel >= NUM_OF_ANALOG_INPUT)
    {
        AD_Channel = (AD_CHANNEL_TYPE)Current_pin;
    }
}

