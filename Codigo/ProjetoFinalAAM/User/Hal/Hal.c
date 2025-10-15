//------------------------ Include Files ----------------------------------
#include "../Header/Hal.h"
#include "../Header/ADC.h"
//------------------------ Global Variables ----------------------------------
// Vari¨¢veis para ADC
ANALOG_INPUT_TYPE AD_Channel;
unsigned short int AD_Accumulator[NUM_OF_ANALOG_INPUT];
unsigned short int AD_Sample[NUM_OF_ANALOG_INPUT];
unsigned char AD_Counter[NUM_OF_ANALOG_INPUT];

unsigned short int Hal_Analog_Inputs[NUM_OF_ANALOG_INPUT];   // buffer da m¨¦dia do valor do ADC de cada canal

//------------------------ Defines, Enumerations ----------------------------------


#define NUM_AD_SAMPLES        8


//------------------------ Private Functions (prototypes) ----------------------------------
void ADProcess(void);


//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

void Hal__Initialize(void){

}
void Hal__BackgroundHandler(void){
    ADProcess();
}
void Hal__FastHandler(void){

}


unsigned short int Hal_GetAnalogInput(ANALOG_INPUT_TYPE input){
    return Hal_Analog_Inputs[input];
}

//=====================================================================================================================
//-------------------------------------- Private Functions -------------------------------------------------------------
//=====================================================================================================================
void ADProcess(void)
{
    // N?o h¨¢ necessidade de muito processamento extra
    if(Adc_Enable_Table[AD_Channel] == SET)
    {
        if(AD_Counter[AD_Channel] < NUM_AD_SAMPLES)
        {
            AD_Sample[AD_Channel] = ADC__GetValue(Adc_Enable_Table[AD_Channel]);
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
        AD_Channel = 0;
    }
}