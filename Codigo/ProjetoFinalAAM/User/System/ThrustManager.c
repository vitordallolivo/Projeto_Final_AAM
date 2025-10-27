#include "../Header/ThrustManager.h"
#include "../Header/SysTick.h"
#include "ch32v00x_usart.h"
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

uint32_t Calib[NUM_OF_LOAD_CELL] = {0};
uint32_t prev_RPM_tick=0;

// ---------------------- Private Functions (prototype) ----------
ERROR_ID TestSensors(void); // check connection of sensor's
ERROR_ID RampDuty(void); // check current, and voltage
uint8_t USART_send_packet(Output_data *packet);
uint8_t USART_receive_packet(Input_data *packet);
void test(float current, float voltage);
// --------------------- Interrupcao ------------------

void CMPInitialize(void){
    RPM_count = 0;
    // Initialize Communication
    USART_Printf_Init(115200);
    memset(&data_out, 0, sizeof(Output_data));
}

void CMP_BackgroundHandler(){

    if(Mode == MOTOR_MODE){

        if(Error_GetStatus(NO_VOLTAGE) && Error_GetStatus(NO_CURRENT) && Error_GetStatus(MOTOR_NOT_SET)){
            Hal_SetMotorDuty(data_in.Duty);
        }
        else{
            Hal_SetMotor(0);
        }
    }
    else{
        
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

    /*
    * Note: The use of floats and doubles has been completely avoided in this code,
    * as the compiler cannot handle these values efficiently. Therefore, all
    * representation is done using integer numbers.
    */


    int32_t Thrust = (GetCellRead(Thrust_Cell) * SCALE_THRUST) / Calib[Thrust_Cell];
    int32_t Torque = (GetCellRead(Torque_Cell) * SCALE_TORQUE) / Calib[Torque_Cell];

    int16_t raw_voltage = (Hal_GetAnalogInput(Voltage_pin) * (5000 / 1023) * Divider_coeff) / 1000;
    int32_t raw_current = ((Hal_GetAnalogInput(Current_pin) * (5000 / 1023) - (int32_t)(QOV * 1000) + 7));
    raw_current = (raw_current * (int32_t)(Sensitivity_Sens * 1000)) / 1000;

    uint16_t RPM = (RPM_count / 2) * (60 * MS_TIMERS_RESOLUTION) / SysTick_GetElapsedTime(prev_RPM_tick);
    prev_RPM_tick = SysTick_GetTick();

    int32_t current, power;
    int16_t voltage;

    current = (raw_current < 0) ? 0 : raw_current;
    voltage = (raw_voltage > (MINIMAL_VOLTAGE * 1000)) ? raw_voltage : 0;
    power = (current * voltage) / 1000;  


    data_out.current = current;
    data_out.voltage = voltage;
    data_out.power = power;
    data_out.Thrust = Thrust;
    data_out.Torque = Torque;
    data_out.RPM = RPM;

}


ERROR_TYPE SupervisionCMP(){
    ERROR_ID err=TestSensors();


    if(err != NONE){
        
    }
    return NUM_ERROR_TYPES;
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
    RPM_count++;
}

//=======================================================================
//                          Private Functions
//=======================================================================

ERROR_ID TestSensors(void){
    return NONE;
}
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

void test(float current, float voltage){
    data_out.current = current;
    data_out.voltage = voltage;
}