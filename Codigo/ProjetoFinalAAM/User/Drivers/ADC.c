#include "ch32v00x_dma.h"
#include "ch32v00x_adc.h"
#include "../Header/ADC.h"



//------------------- Defines, Enumerations ---------------------------------------------


const unsigned char Adc_Enable_Table[NUM_OF_AD_CHANNELS + 1] = { 
    CONFIG_AD_CH0,
    CONFIG_AD_CH1, 
    CONFIG_AD_CH2,
    CONFIG_AD_CH3,
    CONFIG_AD_CH4,
    CONFIG_AD_CH5,  
    CONFIG_AD_CH6,
    CONFIG_AD_CH7,
    CONFIG_AD_CH_VREFINT
};



//----------------------- Variaveis Globais------------------------------------------------

// Buffer do DMA (valores brutos do ADC)
__attribute__((aligned(4))) 
unsigned short int DMA_Buffer[NUM_OF_AD_CHANNELS + 1] = {0};

// Buffer processado (valores filtrados/m谷dia)
unsigned short int ADC_Buffer[NUM_OF_AD_CHANNELS + 1] = {0};



//--------------------- Funcoes Privadas (prototipo)----------------------------//

static void DMA_Tx_Init(DMA_Channel_TypeDef *DMAy_Channelx, uint32_t peripheralAddress, uint32_t memoryAddress, uint16_t bufferSize);
static void ADC_GPIO_Config(void);



// =============================================================================
// ---------------------------FUNCOES PUBLICAS---------------------------------
// =============================================================================

/*******************************************************************************
 * @brief  Inicializa o ADC e DMA
 * @param  None
 * @retval None
 ******************************************************************************/
void ADC__Initialize(void)
{	
    ADC_InitTypeDef ADC_InitStructure = {0};
    uint8_t channel_count = 0;
    uint8_t sequence = 1;

    // Habilita clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(ADC_CLOCK_DIV);

    // Configura GPIOs
    ADC_GPIO_Config();

    // Configura DMA PRIMEIRO
    // Conta quantos canais est?o habilitados
    for(uint8_t i = 0; i < (NUM_OF_AD_CHANNELS + 1); i++) {
        if(Adc_Enable_Table[i] == 1) {
            channel_count++;
        }
    }

    DMA_Tx_Init(DMA1_Channel1, (uint32_t)&ADC1->RDATAR, (uint32_t)DMA_Buffer, channel_count);

    // Configura??o do ADC
    ADC_DeInit(ADC1);
    
    ADC_InitStructure.ADC_Mode = ADC_OPERATION_MODE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ADC_CONTINUOUS_CONV;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DATA_ALIGN;
    ADC_InitStructure.ADC_NbrOfChannel = channel_count;
    
    // Configura sequ那ncia de canais
    for(uint8_t channel = 0; channel < NUM_OF_AD_CHANNELS; channel++) {
        if(Adc_Enable_Table[channel] == 1) {
            ADC_RegularChannelConfig(ADC1, channel, sequence, ADC_SAMPLE_TIME);
            sequence++;
        }
    }
    
    // Configura canal VREFINT se habilitado
    #if CONFIG_AD_CH_VREFINT == 1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, sequence, ADC_SAMPLE_TIME);
    #endif

    ADC_Init(ADC1, &ADC_InitStructure);

    // Habilita DMA do ADC
    ADC_DMACmd(ADC1, ENABLE);

    // Habilita ADC
    ADC_Cmd(ADC1, ENABLE);

    // Calibracao (DEVE ser feita ap車s habilitar o ADC)
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    // Habilita DMA e inicia conversoes
    DMA_Cmd(DMA1_Channel1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/*******************************************************************************
 * @brief  Handler do ADC (chamado na interrup??o do DMA)
 * @param  None
 * @retval None
 ******************************************************************************/
void ADC_Handler(void)
{
    if(DMA_GetFlagStatus(DMA1_FLAG_TC1)) {
        DMA_ClearFlag(DMA1_FLAG_TC1);
        
        // Copia dados do DMA_Buffer para ADC_Buffer
        uint8_t buffer_index = 0;
        for(uint8_t channel = 0; channel < NUM_OF_AD_CHANNELS; channel++) {
            if(Adc_Enable_Table[channel] == 1) {
                ADC_Buffer[channel] = DMA_Buffer[buffer_index];
                buffer_index++;
            }
        }
        
        // Copia VREFINT se habilitado
        #if CONFIG_AD_CH_VREFINT == 1
        ADC_Buffer[NUM_OF_AD_CHANNELS] = DMA_Buffer[buffer_index];
        #endif
    }
}

/*******************************************************************************
 * @brief  Configura modos do ADC
 * @param  mode: Modo de opera??o (8 ou 10 bits)
 * @param  conv_type: Tipo de convers?o (single/continuous)
 * @retval None
 ******************************************************************************/
void ADC__ConfigModes(AD_MODE_TYPE mode, AD_CONVERSION_TYPE conv_type)
{
    ADC_InitTypeDef ADC_InitStructure = {0};

    // Configura alinhamento baseado no modo
    if(mode == MODE_8BITS) {
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
    } else {
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    }

    // Configura modo de convers?o
    if(conv_type == AD_SINGLE_CONVERSION) {
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    } else {
        ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    }

    ADC_InitStructure.ADC_Mode = ADC_OPERATION_MODE;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    
    // Mant谷m o n迆mero de canais configurado
    uint8_t channel_count = 0;
    for(uint8_t i = 0; i < (NUM_OF_AD_CHANNELS + 1); i++) {
        if(Adc_Enable_Table[i] == 1) {
            channel_count++;
        }
    }
    ADC_InitStructure.ADC_NbrOfChannel = channel_count;

    ADC_Init(ADC1, &ADC_InitStructure);
}

/*******************************************************************************
 * @brief  Obt谷m valor bruto do ADC para um canal espec赤fico
 * @param  channel: Canal ADC
 * @retval Valor bruto do ADC (0-1023)
 ******************************************************************************/
unsigned short int ADC__GetValue(AD_CHANNEL_TYPE channel)
{
    if(channel < NUM_OF_AD_CHANNELS) {
        return ADC_Buffer[channel];
    }
    return 0;
}



// =============================================================================
// --------------------------FUNCOES PRIVADAS-------------------------------------
// =============================================================================



/*******************************************************************************
 * @brief  Inicializa o DMA para o ADC
 * @param  DMA_CHx: Canal do DMA
 * @param  peripheralAddress: Endereco do perif谷rico (ADC)
 * @param  memoryAddress: Endereco da mem車ria (buffer)
 * @param  bufferSize: Tamanho do buffer
 * @retval None
 ******************************************************************************/
static void DMA_Tx_Init(DMA_Channel_TypeDef *DMAy_Channelx, uint32_t peripheralAddress, 
                       uint32_t memoryAddress, uint16_t bufferSize)
{
    uint32_t tmpreg = 0;

    // Habilita clock do DMA (usando registro direto)
    RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

    // 1. Configura CFGR (Control Register)
    tmpreg = DMAy_Channelx->CFGR;
    tmpreg &= ~(DMA_CFGR1_EN | DMA_CFGR1_MEM2MEM); // Clear enable and M2M bits
    
    // Configura todos os bits de uma vez (igual na fun??o original)
    tmpreg |= DMA_DIR_PeripheralSRC    |  // Dire??o: Peripheral -> Memory
              DMA_Mode_Circular        |  // Modo circular  
              DMA_PeripheralInc_Disable | // Peripheral no increment
              DMA_MemoryInc_Enable     |  // Memory increment enable
              DMA_PeripheralDataSize_HalfWord | // 16 bits peripheral
              DMA_MemoryDataSize_HalfWord |    // 16 bits memory
              DMA_Priority_High;           // Prioridade alta

    DMAy_Channelx->CFGR = tmpreg;
    
    // 2. Configura os outros registradores diretamente
    DMAy_Channelx->CNTR = bufferSize;           // Tamanho do buffer
    DMAy_Channelx->PADDR = peripheralAddress;   // Endere?o do perif谷rico
    DMAy_Channelx->MADDR = memoryAddress;       // Endere?o da mem車ria
}

/*******************************************************************************
 * @brief  Configura os pinos GPIO para os canais ADC habilitados
 * @param  None
 * @retval None
 ******************************************************************************/
static void ADC_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Configura cada canal habilitado conforme o mapeamento de pinos
    #if CONFIG_AD_CH0 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // PA2 - Channel 0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH1 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // PA1 - Channel 1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH2 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; // PC4 - Channel 2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH3 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // PD2 - Channel 3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH4 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // PD3 - Channel 4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH5 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; // PD5 - Channel 5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH6 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // PD6 - Channel 6
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    #endif

    #if CONFIG_AD_CH7 == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; // PD4 - Channel 7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    #endif
}
