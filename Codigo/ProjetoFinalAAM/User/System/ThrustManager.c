#include "../Header/ThrustManager.h"
#include "../Header/Comunication.h"
#include "../Header/SysTick.h"
#include "../Header/Sound.h"
#include "Header/PPM.h"
#include "ch32v00x_dma.h"
#include "ch32v00x_usart.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
// ---------------------- Defines, Enumeration ---------------
typedef enum {
    WAIT_SYNC,
    RECEIVING_DATA,
    RECEIVE_CHECKSUM,
    RECEIVE_TERMINATOR
} ReceiverState;

//----------------------- Global Variables ---------------------

uint32_t RPM_count = 0;

Input_data data_in = {0};
Output_data data_out ={0};
uint8_t Mode = 0;
uint16_t Duty = 0;

uint32_t Calib[NUM_OF_LOAD_CELL] = {1};

uint32_t PWM_tick = 0;
uint16_t LenghtPWM = 0;
uint32_t LastPWM_tick = 0;

uint32_t prev_RPM_tick=0;
uint16_t rho0 = 1225; // Densidade do ar

char Rxbuffer[64] = {0};
u32 RxBuffer1_Index = 0;



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

    GetInputData(&data_in);

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
}

void CMP_BackgroundHandler() {
   
    /*
    * Note: The use of floats and doubles has been completely avoided in this code,
    * as the compiler cannot handle these values efficiently. Therefore, all
    * representation is done using integer numbers.
    */

    int32_t Thrust = (GetCellRead(Thrust_Cell)) / Calib[Thrust_Cell];
    int32_t Torque = (GetCellRead(Torque_Cell)) / Calib[Torque_Cell];

    if(Thrust < 0 || Thrust > 50){
        Error_Detect(ERROR_LOAD_CELL,LOAD_CELL_NOT_CALIBRATED);
    }else{
        Error_Erase(ERROR_LOAD_CELL,LOAD_CELL_NOT_CALIBRATED);
    }
    
    if(Thrust > 0xffff || Torque > 0xffff ) {
        Error_Detect(ERROR_LOAD_CELL,LOAD_CELL_NOT_CONNECTED);
    }else{
        Error_Erase(ERROR_LOAD_CELL,LOAD_CELL_NOT_CONNECTED);

    }

    int32_t raw_voltage = (((SCALE_VOLTAGE*ADC_REF_VOLTAGE)*Hal_GetAnalogInput(Voltage_pin)*VOLTAGE_COEFF)/ADC_RESOLUTION);
    int32_t raw_current = ((SCALE_CURRENT*(ADC_REF_VOLTAGE*Hal_GetAnalogInput(Current_pin))/ADC_RESOLUTION)-CURRENT_OFFSET)*CURRENT_COEFF;
    int32_t raw_pressure = (SCALE_PRESSURE * ADC_REF_VOLTAGE * (Hal_GetAnalogInput(Pitot_pin)) / (ADC_RESOLUTION*PITOT_COEFF));
    
    if(raw_pressure < MINIMAL_PITOT){
        Error_Detect(ERROR_PITOT,PITOT_NOT_ENGAGED);
    }else{
        Error_Erase(ERROR_PITOT,PITOT_NOT_ENGAGED);
    }

    uint32_t elapsed_time = SysTick_GetElapsedTime(prev_RPM_tick);


    int64_t RPM = (RPM_count / 2) * (60 * MS_TIMERS_RESOLUTION) / elapsed_time;
    prev_RPM_tick = SysTick_GetTick();

    int32_t current, power;
    int32_t voltage;

    if(raw_current < 0){
        current = 0;
        Error_Detect(ERROR_MOTOR,NO_CURRENT);
    }
    else{
        current = raw_current;
        Error_Erase(ERROR_MOTOR,NO_CURRENT);
    }

    if(raw_voltage > MINIMAL_VOLTAGE){
        voltage = raw_voltage;
        Error_Erase(ERROR_MOTOR,NO_VOLTAGE);
    }
    else{
        voltage = 0;
        Error_Detect(ERROR_MOTOR,NO_VOLTAGE);
    }

    // C?lculo seguro de power (evitar overflow)
    int64_t big_power = (int64_t)current * voltage;
    power = big_power / 1000;

    if(LenghtPWM > MINIMAL_PWM && LenghtPWM < MAX_PWM){
        data_out.Duty = (uint16_t)((uint32_t)(((uint32_t)LenghtPWM - 89)));
        Error_Erase(ERROR_MOTOR,RC_RECEIVER_FAILED);
    } else {
        Error_Detect(ERROR_MOTOR,RC_RECEIVER_FAILED);
    }


    data_out.current = current;
    data_out.voltage = voltage;
    data_out.power = power;
    data_out.Thrust =Thrust;
    data_out.Torque =Torque;
    data_out.RPM = RPM;
    

    int32_t pressure_for_sqrt = (2 * raw_pressure) / rho0;
    if(pressure_for_sqrt < 0) pressure_for_sqrt = 0;
    int32_t velocity_temp = newton_sqrt(pressure_for_sqrt);

    data_out.velocity = velocity_temp;


    if(data_out.Duty > data_in.Duty){
        Duty = data_out.Duty;
    }
    else{
        Duty = data_in.Duty;
    }
}

void ThrustManager_SetMotorAction(MotorAction act){
    if(act == Turn_off){
        PPM_SetValue(Duty);
    }
    else{
        PPM_SetValue(Duty);
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
        
        case NOTHING_MODE:
        break;

        case RESET_MODE:
            NVIC_SystemReset();
        break;

        default:
        break;
    }
}


void Thrust_UpdateData(void){
    static char buffer[64];
    sprintf(buffer,"%d;%d;%d;%d;%d;%d;%d;%d;%d.\r\n",data_out.current,data_out.voltage,data_out.power,data_out.Thrust,data_out.Torque,data_out.RPM,data_out.velocity,Duty,data_out.Err_table);
    SendData((void*)buffer,64);
}


//=======================================================================
//                          Private Functions
//=======================================================================




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




void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void){

    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
        RPM_count++;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    
    if(EXTI_GetITStatus(EXTI_Line2) != RESET) {
        PWM_tick = SysTick_GetTick();
        if(PWM_tick>LastPWM_tick){
            if((PWM_tick - LastPWM_tick) < 500){
                LenghtPWM = PWM_tick - LastPWM_tick;
            }
            LastPWM_tick = PWM_tick;

        }
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

