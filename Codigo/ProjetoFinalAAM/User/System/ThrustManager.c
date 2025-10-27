#include "../Header/ThrustManager.h"
#include "../Header/SysTick.h"
#include "../Header/Sound.h"
#include "ch32v00x_usart.h"
#include <math.h>
#include <string.h>
// ---------------------- Defines, Enumeration ---------------
typedef enum {
    WAIT_SYNC,
    RECEIVING_DATA,
    RECEIVE_CHECKSUM,
    RECEIVE_TERMINATOR
} ReceiverState;

typedef struct __attribute__((packed)){ // Output Data
    uint32_t current;
    uint16_t voltage;
    uint32_t power;
    uint32_t Thrust;
    uint32_t Torque;
    uint16_t RPM;
    uint32_t velocity; // 10 vezes a velocidae
    uint16_t Duty;
    uint8_t Err_table;
}Output_data;


typedef struct{
    uint8_t Mode; // Calibration Mode, or Active Mode 
    uint32_t CalibrationFactorThrust;
    uint32_t CalibrationFactorTorque;
    uint16_t Duty;
}Input_data;



//----------------------- Global Variables ---------------------

uint32_t RPM_count = 0;

Input_data data_in = {0};
Output_data data_out ={0};
uint8_t Mode = 0;
uint8_t Duty = 0;

uint32_t Calib[NUM_OF_LOAD_CELL] = {0};

uint32_t PWM_tick = 0;
uint16_t LenghtPWM = 0;
uint32_t LastPWM_tick = 0;

uint32_t prev_RPM_tick=0;
uint16_t rho0 = 1225; // Densidade do ar

// ---------------------- Private Functions (prototype) ----------
ERROR_ID RampDuty(void); // check current, and voltage
int32_t newton_sqrt(int32_t x);
// --------------------- Interrupcao ------------------


// ==============================================================
//                      Public Functions
// ==============================================================

void CheckCriticalError(void){
    ERROR_ID id;

    data_out.Err_table = 0x00;

    for (id = NO_VOLTAGE; id < NUM_ERRORS; id++) {
        data_out.Err_table|=(Error_GetStatus(id)<<(id-1)) ; // Escreve na tabela de erros
    }

    if(data_out.Err_table & (1<<(NO_VOLTAGE-1))){
        Sounds__PlaySounds(PLAY_ERROR,NO_VOLTAGE);
    }
    else {
        if(data_out.Err_table & (1<<(NO_CURRENT-1))){
            Sounds__PlaySounds(PLAY_ERROR,NO_CURRENT);
        }
        else{
            Sounds__PlaySounds(PLAY_NO_SOUND,NONE);
        }   
    }
    
}

UserAction SupervisionCMP(void){
    

    switch (data_in.Mode) {
        case NOTHING_MODE:
            return NO_EVENT;
        break;
        case CALIBRATION_MODE:
            return Configuration;
        break;

        case TARE_MODE:
            return Configuration;
        break;

        case MOTOR_MODE:
            return Released_Action;
        break;

        default:
            return NO_EVENT;
            break;
    }
}

void CMPInitialize(void){
    RPM_count = 0;
    // Initialize Communication
    USART_Printf_Init(115200);
    
    // Clear the packet's
    memset(&data_out, 0, sizeof(Output_data));
    memset(&data_in, 0, sizeof(Input_data));

    DMA_InitTypeDef DMA_InitStructure;
    
    // Habilita clock do DMA
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // Configura DMA para RX (Canal 5)
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&data_in;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = sizeof(Input_data);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    
    // Configura DMA para TX (Canal 4)
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&data_out;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = sizeof(Output_data);
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
    
    // Habilita interrup??es
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    
    // Habilita os canais
    DMA_Cmd(DMA1_Channel5, ENABLE);
    // DMA TX s? habilita quando for transmitir
    
    // Configura USART para usar DMA
    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
    
}

void CMP_BackgroundHandler(){


    /*
    * Note: The use of floats and doubles has been completely avoided in this code,
    * as the compiler cannot handle these values efficiently. Therefore, all
    * representation is done using integer numbers.
    */


    int32_t Thrust = (GetCellRead(Thrust_Cell)) / Calib[Thrust_Cell];
    int32_t Torque = (GetCellRead(Torque_Cell)) / Calib[Torque_Cell];

    if(Thrust < 0 || Thrust > 50){
        Error_Detect(ERROR_LOAD_CELL,LOAD_CELL_NOT_CALIBRATED);
    }
    if(Thrust > 0xffff || Torque > 0xffff ) {
        Error_Detect(ERROR_LOAD_CELL,LOAD_CELL_NOT_CONNECTED);
    }

    int16_t raw_voltage = (((SCALE_VOLTAGE*ADC_REF_VOLTAGE)*Hal_GetAnalogInput(Voltage_pin)*VOLTAGE_COEFF)/ADC_RESOLUTION);
    int32_t raw_current = ((SCALE_CURRENT*(ADC_REF_VOLTAGE*Hal_GetAnalogInput(Current_pin))/ADC_RESOLUTION)-CURRENT_OFFSET)*CURRENT_COEFF;
    int32_t raw_pressure = (SCALE_PRESSURE * ADC_REF_VOLTAGE * (Hal_GetAnalogInput(Pitot_pin)) / (ADC_RESOLUTION*PITOT_COEFF));
    
    if(raw_pressure < MINIMAL_PITOT){
        Error_Detect(ERROR_PITOT,PITOT_NOT_ENGAGED);
    }

    uint16_t RPM = (RPM_count / 2) * (60 * MS_TIMERS_RESOLUTION) / SysTick_GetElapsedTime(prev_RPM_tick);
    prev_RPM_tick = SysTick_GetTick();

    int32_t current, power;
    int16_t voltage;

    if(raw_current < 0){
        current = 0;
        Error_Detect(ERROR_MOTOR,NO_CURRENT);
    }
    else{
        current = raw_current;
    }

    if(raw_voltage > MINIMAL_VOLTAGE){
        voltage = raw_voltage;
    }
    else{
        voltage = 0;
        Error_Detect(ERROR_MOTOR,NO_VOLTAGE);
    }
    power = (current * voltage) / 1000;

    if(LenghtPWM>MINIMAL_PWM && LenghtPWM<MAX_PWM){
        data_out.Duty = LenghtPWM/2 -MINIMAL_PWM;
    }else{
        Error_Detect(ERROR_MOTOR,RC_RECEIVER_FAILED);
    }


    data_out.current = current;
    data_out.voltage = voltage;
    data_out.power = power;
    data_out.Thrust = Thrust;
    data_out.Torque = Torque;
    data_out.RPM = RPM;
    data_out.velocity = newton_sqrt((2 * raw_pressure) / rho0);

    if(data_out.Duty > data_in.Duty){
        Duty = data_out.Duty;
    }
    else{
        Duty = data_in.Duty;
    }
}


void ThrustManager_SetMotorAction(MotorAction act){
    if(act == Turn_off){
        Hal_SetMotor(0);
    }
    else{
        Hal_SetMotorDuty(Duty);
    }
}

void ThrustManager_SetLoadCell(){
    
    switch (data_in.Mode) {
        case CALIBRATION_MODE:
            Calib[Thrust_Cell]=data_in.CalibrationFactorThrust;
            Calib[Torque_Cell]=data_in.CalibrationFactorTorque;
            data_in.Mode = NOTHING_MODE;
        break;

        case TARE_MODE:
            LoadCellTare(Thrust_Cell);
            LoadCellTare(Torque_Cell);
            data_in.Mode = NOTHING_MODE;
        break;
        
        default:
        break;
    }
}





void EXTI7_0_IRQHandler(void){

    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
        RPM_count++;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    
    if(EXTI_GetITStatus(EXTI_Line2) != RESET) {
        PWM_tick = SysTick_GetTick();
        if(PWM_tick>LastPWM_tick){
            LenghtPWM = PWM_tick - LastPWM_tick;
            LastPWM_tick = PWM_tick;
        }
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

//=======================================================================
//                          Private Functions
//=======================================================================


ERROR_ID RampDuty(void){
    return NONE;
}


int32_t newton_sqrt(int32_t x){
    if (x <= 0) return 0;
    
    // Escala para precis?o
    int64_t scaled_x = (int64_t)x * 100;
    int32_t r = x / 2;
    if (r <= 0) r = 1;
    
    for (int i = 0; i < 10; i++) {
        int32_t r2 = (r + (scaled_x / r)) / 2;
        if (r2 == r) break;
        r = r2;
    }
    
    return r;
}