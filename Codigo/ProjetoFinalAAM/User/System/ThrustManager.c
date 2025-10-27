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


#define SIZE_OF_OUTPUT_DATA sizeof(Output_data)
#define SIZE_OF_INPUT_DATA sizeof(Input_data)



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
uint16_t rho0 = 1225; // era para ser 1.225 kg



// ---------------------- Private Functions (prototype) ----------
ERROR_ID RampDuty(void); // check current, and voltage
uint8_t USART_send_packet(Output_data *packet);
uint8_t USART_receive_packet(Input_data *packet);
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
        data_out.Duty = LenghtPWM;
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
        Hal_SetMotor(Duty);
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


void CommunitacionHandle(void){

    static unsigned char transmitting = false;
    static unsigned char receiving = false;

    if(Timer__MsGetStatus(TIMER_MS_COMMUNICATION) == TIMER_EXPIRED){
        transmitting = true;
        receiving = true;
        Timer__MsSet(TIMER_MS_COMMUNICATION,10000); 
    }

    if(transmitting == true){
        if(USART_send_packet(&data_out) == true){
            transmitting = false;
        }

    }

    if(receiving == true){
        while(USART_receive_packet(&data_in)!=true);
        receiving = false;
    }


    return;
}




void EXTI7_0_IRQHandler(void){

    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // Sua l¨®gica para interrup??o do PD0 (borda de subida)
        RPM_count++;
        // Limpar flag de interrup??o
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    
    // Verificar se EXTI2 (PD2) gerou a interrup??o
    if(EXTI_GetITStatus(EXTI_Line2) != RESET) {
        PWM_tick = SysTick_GetTick();
        if(PWM_tick>LastPWM_tick){
            LenghtPWM = PWM_tick - LastPWM_tick;
            LastPWM_tick = PWM_tick;
        }

        // Limpar flag de interrup??o
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

//=======================================================================
//                          Private Functions
//=======================================================================


ERROR_ID RampDuty(void){
    return NONE;
}

uint8_t USART_send_packet(Output_data *packet){
    static uint8_t byte_count = 0;
    static uint8_t checksum = 0;

    uint8_t* data_bytes = (uint8_t*)packet;

    if(byte_count == 0){ // Primeira etapa
        checksum = 0; // Reset o checksum
        USART_SendData(USART1,SYNC_BYTE1); // manda um byte de sincroniza??o
    }
    checksum += data_bytes[byte_count];
    USART_SendData(USART1,data_bytes[byte_count]);
    byte_count++;
            
    if (byte_count >= SIZE_OF_OUTPUT_DATA) { // Mensagem acabou
        USART_SendData(USART1,checksum); // Manda o Check sum
        USART_SendData(USART1,'\n'); // Terminador
        byte_count = 0; // reseta

        return true; // acabou o processo
    }

    return false; // N?o acabou
}

uint8_t USART_receive_packet(Input_data *packet) {
    static ReceiverState state = WAIT_SYNC;
    static uint8_t byte_count = 0;
    static uint8_t checksum = 0;
    static uint8_t temp_buffer[SIZE_OF_INPUT_DATA];
    
    uint8_t received_byte = USART_ReceiveData(USART1);
    
    switch(state) {
        case WAIT_SYNC:
            if(received_byte == SYNC_BYTE1) {
                state = RECEIVING_DATA;
                byte_count = 0;
                checksum = 0;
            }
            break;

        case RECEIVING_DATA:
            temp_buffer[byte_count] = received_byte;
            checksum += received_byte;
            byte_count++;
            
            if(byte_count >= SIZE_OF_INPUT_DATA) {
                state = RECEIVE_CHECKSUM;
            }
            break;

        case RECEIVE_CHECKSUM:
            if(checksum == received_byte) {
                state = RECEIVE_TERMINATOR;
            } else {
                state = WAIT_SYNC;
            }
            break;

        case RECEIVE_TERMINATOR:
            state = WAIT_SYNC;
            if(received_byte == '\n') {
                // EVITE C?PIA - use memcpy ou atribui??o direta campo a campo
                *packet = *(Input_data*)temp_buffer;
                return true;
            }
            break;
    }
    
    return false;
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