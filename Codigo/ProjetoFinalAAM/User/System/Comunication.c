#include "../Header/Comunication.h"
#include <string.h>

#define TxSize1    (size(TxBuffer1))
#define size(a)    (sizeof(a) / sizeof(*(a)))

u8 TxBuffer1[64];
u8 RxBuffer1[TxSize1] = {0};


void USARTx_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    DMA_Cmd(DMA1_Channel4, ENABLE); /* USART1 Tx */
    DMA_Cmd(DMA1_Channel5, ENABLE); /* USART1 Rx */

    USART_Cmd(USART1, ENABLE);
}

void DMA_INIT(void)
{
    DMA_InitTypeDef DMA_InitStructure = {0};
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = TxSize1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // Alterado para Normal
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    // Mantenha o RX como circular se quiser
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = TxSize1;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
}


void Comunication_Init(void){
    DMA_INIT();
    USARTx_CFG();
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}


void SendNewMessage(const char* message)
{
    // Espera a transmiss?o anterior terminar
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET){
    }
    
    // Limpa a flag
    DMA_ClearFlag(DMA1_FLAG_TC4);
    
    // Copia a nova mensagem para o buffer
    memset(TxBuffer1, 0, sizeof(TxBuffer1));
    strncpy((char*)TxBuffer1, message, sizeof(TxBuffer1) - 1);
    
    // Reconfigura e reinicia o DMA
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA1_Channel4->CNTR = strlen((char*)TxBuffer1);
    DMA_Cmd(DMA1_Channel4, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}




void SendData(void* data, uint16_t size){
    if(data == NULL || size == 0) {
        return;
    }
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET){
      
    }
    DMA_ClearFlag(DMA1_FLAG_TC4);

    memcpy(TxBuffer1,data,size);

    DMA_ClearFlag(DMA1_FLAG_TC4);
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA1_Channel4->CNTR = strlen((char*)TxBuffer1);
    DMA_Cmd(DMA1_Channel4, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    
}

// Recebe dados se dispon¨ªveis
u8 ReceiveData(char* data, uint16_t size) {
    if(data == NULL || size == 0) {
        return 0;
    }
    
    if(DMA_GetFlagStatus(DMA1_FLAG_TC5) == RESET) {
        return 0;
    }

    // ?? DESLIGA o DMA antes de mexer no buffer
    DMA_Cmd(DMA1_Channel5, DISABLE);
    
    // Calcula bytes recebidos
    uint16_t bytesRecebidos = TxSize1 - DMA_GetCurrDataCounter(DMA1_Channel5);
    
    if(bytesRecebidos == 0) {
        // Se n?o tem dados, s? reinicia o DMA
        DMA1_Channel5->CNTR = TxSize1;
        DMA_Cmd(DMA1_Channel5, ENABLE);
        DMA_ClearFlag(DMA1_FLAG_TC5);
        return 0;
    }
    
    // Limita c?pia
    if(bytesRecebidos > size - 1) {
        bytesRecebidos = size - 1;
    }
    
    // Copia dados
    memcpy(data, RxBuffer1, bytesRecebidos);
    data[bytesRecebidos] = '\0';
    
    printf("?? Recebido: %d bytes -> %s\n", bytesRecebidos, data);
    
    // ?? LIMPA o buffer do RxBuffer1 (agora ? seguro)
    memset(RxBuffer1, 0, TxSize1);
    
    // ?? REINICIA o DMA
    DMA1_Channel5->CNTR = TxSize1;  // Reseta o contador
    DMA_Cmd(DMA1_Channel5, ENABLE);  // Liga o DMA novamente
    DMA_ClearFlag(DMA1_FLAG_TC5);    // Limpa a flag
    
    return bytesRecebidos;
}