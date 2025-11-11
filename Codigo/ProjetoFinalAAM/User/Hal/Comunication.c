#include "../Header/Comunication.h"

#include <string.h>
// ---------------------- Defines, Enumeration ---------------

#define TxSize1    (size(TxBuffer1))
#define RxSize1    (sizeof(RxBuffer1))
#define size(a)    (sizeof(a) / sizeof(*(a)))

#define NEW_DATA_AVAILABLE 1
#define NEW_DATA_NOT_AVAILABLE 0
//----------------------- Global Variables ---------------------

u8 TxBuffer1[64];
u8 RxBuffer1[TxSize1] = {0};
u8 NewData = 0;
Input_data data_buffer = {0};

// ---------------------- Private Functions (prototype) ----------

u8 find_pattern(const char *str, int n, int *values);


// ==============================================================
//                      Public Functions
// ==============================================================

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

void GetInputData(Input_data *data) {

    if(NewData !=NEW_DATA_AVAILABLE){
        return;
    }

    data->Mode = data_buffer.Mode;
    data->CalibrationFactorThrust = data_buffer.CalibrationFactorThrust;
    data->CalibrationFactorTorque = data_buffer.CalibrationFactorTorque;
    data->Duty = data_buffer.Duty;
}


u8 ReceiveData(char* data, uint16_t size) {
    if(data == NULL || size == 0) {
        return 0;
    }
    
    if(DMA_GetFlagStatus(DMA1_FLAG_TC5) == RESET) {
        return 0;
    }


    DMA_Cmd(DMA1_Channel5, DISABLE);
    
    // Calcula bytes recebidos
    uint16_t bytesRecebidos = TxSize1 - DMA_GetCurrDataCounter(DMA1_Channel5);
    
    if(bytesRecebidos == 0) {

        NewData = NEW_DATA_NOT_AVAILABLE;

        // Se nao tem dados, reinicia o DMA
        DMA1_Channel5->CNTR = TxSize1;
        DMA_Cmd(DMA1_Channel5, ENABLE);
        DMA_ClearFlag(DMA1_FLAG_TC5);
        return 0;
    }
    

    NewData = NEW_DATA_AVAILABLE;

    // Limita copia
    if(bytesRecebidos > size - 1) {
        bytesRecebidos = size - 1;
    }
    
    // Copia dados
    memcpy(data, RxBuffer1, bytesRecebidos);
    data[bytesRecebidos] = '\0';
    
    memset(RxBuffer1, 0, TxSize1);
    

    DMA1_Channel5->CNTR = TxSize1;  // Reseta o contador
    DMA_Cmd(DMA1_Channel5, ENABLE);  // Liga o DMA novamente
    DMA_ClearFlag(DMA1_FLAG_TC5);    // Limpa a flag
    
    return bytesRecebidos;
}

void ProcessReceivedData(void) {
    static u8 processing_buffer[RxSize1];
    u8 bytes_received;
    
    // Copy received data to processing buffer
    bytes_received = ReceiveData((char*)processing_buffer, RxSize1);
    
    if (bytes_received > 0) {
        // Try to find pattern in the received data
        int values[4] = {0};
        
        if (find_pattern((const char*)processing_buffer, bytes_received, values)) {
            // Pattern found - update data_buffer
            data_buffer.Mode = (uint8_t)values[0];
            data_buffer.CalibrationFactorThrust = values[1];        // Second value as Duty
            data_buffer.CalibrationFactorTorque = values[2]; // Third value as Thrust Calibration
            data_buffer.Duty = (uint16_t)values[3]; // Fourth value as Torque Calibration
        }
    }
}


//=======================================================================
//                          Private Functions
//=======================================================================



u8 find_pattern(const char *str, int n, int *values) {
    int i = 0;
    int state = 0;
    int num_idx = 0;
    int value = 0;
    int has_digits = 0;
    
    for (i = 0; i < n; i++) {
        char c = str[i];
        
        switch (state) {
            case 0:
                if (c == '0' && i + 4 < n && 
                    str[i+1] == 'x' && str[i+2] == '7' && 
                    str[i+3] == '3' && str[i+4] == '.') {
                    state = 1;
                    num_idx = 0;
                    value = 0;
                    has_digits = 0;
                    i += 4;
                }
                break;
                
            case 1:
                if (c >= '0' && c <= '9') {
                    value = value * 10 + (c - '0');
                    has_digits = 1;
                } 
                else if (c == ';' || c == '.') {
                    if (has_digits) {
                        values[num_idx++] = value;
                        value = 0;
                        has_digits = 0;
                        
                        if (c == '.') {
                            if (i + 6 < n && 
                                str[i+1] == '0' && str[i+2] == 'x' && 
                                str[i+3] == '8' && str[i+4] == '4' && 
                                str[i+5] == '\r' && str[i+6] == '\n') {
                                if (num_idx == 4) return 1;
                            }
                            state = 0;
                        }
                    } else {
                        state = 0;
                    }
                }
                else {
                    state = 0;
                }
                break;
        }
    }
    return 0;
}