#ifndef THRUST_MANAGER_PRM_H
#define THRUST_MANAGER_PRM_H


// Modos do ThrustManager
#define NOTHING_MODE 0x00
#define CALIBRATION_MODE 0xff
#define TARE_MODE 0x0f
#define MOTOR_MODE 0xAF

#define SYNC_BYTE1 0xAA




// Sensores

#define ADC_REF_VOLTAGE 5 // 5V
#define ADC_RESOLUTION 1023

#define SCALE_THRUST 1000      // mili - kgf
#define SCALE_TORQUE 1000      // mili - kgf-cm
#define SCALE_VOLTAGE 1000     // milli-volts
#define SCALE_CURRENT 1000     // milli-amps
#define SCALE_PRESSURE 1000    // Pa
#define SCALE_POWER 1000       // milli-watts

#define MINIMAL_VOLTAGE 2500 // 2.5V
#define MINIMAL_CURRENT 0000 // 0.5 A
#define MINIMAL_PITOT -2000

#define VOLTAGE_COEFF 6 // Resistor de 10k em serie com 2k

#define CURRENT_COEFF 50 // ACS758LCB-100B-PFF-T
#define CURRENT_OFFSET 2500

#define PITOT_COEFF 6 // O ganho do sensor ? 1V/kPa, porem por conta do circuito para adequar o sinal
#define PITOT_OFFSET 0




#endif