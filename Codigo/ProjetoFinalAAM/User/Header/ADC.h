#ifndef ADC_H
#define ADC_H

#include "ch32v00x.h"
#include "ADC_prm.h"

// =============================================================================
// DEFINI??ES P?BLICAS
// =============================================================================


// Tipos de canais ADC
typedef enum {
    AD_CH0 = 0,
    AD_CH1,
    AD_CH2, 
    AD_CH3,
    AD_CH4,
    AD_CH5,
    AD_CH6,
    AD_CH7,
    NUM_OF_AD_CHANNELS,
    AD_CH_VREFINT = 0x1E  // Canal interno da refer¨ºncia
} AD_CHANNEL_TYPE;

// Modos de opera??o do ADC
typedef enum {
    MODE_8BITS = 0,
    MODE_10BITS,
    NUM_OF_MODES
} AD_MODE_TYPE;

// Tipos de convers?o
typedef enum {
    AD_SINGLE_CONVERSION = 0,
    AD_CONTINUOUS_CONVERSION
} AD_CONVERSION_TYPE;

// =============================================================================
// VARI?VEIS P?BLICAS
// =============================================================================

extern unsigned short int ADC_Buffer[NUM_OF_AD_CHANNELS + 1]; // +1 para VREFINT
extern const unsigned char Adc_Enable_Table[NUM_OF_AD_CHANNELS + 1];

// =============================================================================
// PROT?TIPOS DAS FUN??ES P?BLICAS
// =============================================================================

void ADC__Initialize(void);
void ADC_Handler(void);
void ADC__ConfigModes(AD_MODE_TYPE mode, AD_CONVERSION_TYPE conv_type);
unsigned short int ADC__GetValue(AD_CHANNEL_TYPE channel);

#endif