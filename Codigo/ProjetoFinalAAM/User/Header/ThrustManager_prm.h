#ifndef THRUST_MANAGER_PRM_H
#define THRUST_MANAGER_PRM_H


// Modos do ThrustManager
#define NOTHING_MODE 0x00
#define CALIBRATION_MODE 0xff
#define TARE_MODE 0x0f
#define MOTOR_MODE 0xAF

#define SYNC_BYTE1 0xAA

// Sensores


#define Divider_coeff 12/2 // Resistor de 10k em serie com 2k
#define Sensitivity_Sens 1/(20e-3) // ACS758LCB-100B-PFF-T
#define Quiescent_voltage 0.5f
#define MINIMAL_VOLTAGE 2.5f
#define QOV Quiescent_voltage*5.0f
#define Sensor_cutoff (1/Sensitivity_Sens)*cutofflimit



#endif