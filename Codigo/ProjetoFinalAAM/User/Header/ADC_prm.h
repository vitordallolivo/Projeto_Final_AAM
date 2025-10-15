#ifndef ADC_PRM_H
#define ADC_PRM_H

// =============================================================================
// CONFIGURA??O DOS CANAIS ADC
// =============================================================================

// Defina como 1 para habilitar o canal, 0 para desabilitar
#define CONFIG_AD_CH0                0  // PA2 - Canal 0
#define CONFIG_AD_CH1                0  // PA1 - Canal 1  
#define CONFIG_AD_CH2                1  // PC4 - Canal 2
#define CONFIG_AD_CH3                1  // PD2 - Canal 3
#define CONFIG_AD_CH4                0  // PD3 - Canal 4
#define CONFIG_AD_CH5                0  // PD5 - Canal 5
#define CONFIG_AD_CH6                0  // PD6 - Canal 6
#define CONFIG_AD_CH7                0  // PD4 - Canal 7

// Canal interno da refer¨ºncia de tens?o (1 para habilitar)
#define CONFIG_AD_CH_VREFINT         1

// =============================================================================
// CONFIGURA??ES GERAIS DO ADC
// =============================================================================

// Tempo de amostragem (escolha um):
// ADC_SampleTime_1Cycles5    - 1.5 ciclos
// ADC_SampleTime_7Cycles5    - 7.5 ciclos  
// ADC_SampleTime_13Cycles5   - 13.5 ciclos
// ADC_SampleTime_28Cycles5   - 28.5 ciclos
// ADC_SampleTime_41Cycles5   - 41.5 ciclos
// ADC_SampleTime_55Cycles5   - 55.5 ciclos
// ADC_SampleTime_71Cycles5   - 71.5 ciclos
// ADC_SampleTime_239Cycles5  - 239.5 ciclos
#define ADC_SAMPLE_TIME              ADC_SampleTime_241Cycles

// Divisor do clock do ADC
#define ADC_CLOCK_DIV                RCC_PCLK2_Div8

// Modo de opera??o do ADC
#define ADC_OPERATION_MODE           ADC_Mode_Independent

// Alinhamento dos dados
#define ADC_DATA_ALIGN               ADC_DataAlign_Right

// Modo de convers?o
#define ADC_CONTINUOUS_CONV          ENABLE

// =============================================================================
// CONFIGURA??O DO DMA
// =============================================================================

// Modo do DMA
#define DMA_MODE                     DMA_Mode_Circular

// Prioridade do DMA
#define DMA_PRIORITY                 DMA_Priority_VeryHigh

#endif