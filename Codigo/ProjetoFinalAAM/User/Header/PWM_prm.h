#ifndef PWM_PRM_H_
#define PWM_PRM_H_

#include "ch32v00x.h"

#define DISABLED    0
#define ENABLED     1

// PWM channel configuration - ONLY EXISTING PINS ON CH32V003F4P6
#define CONFIG_PWM0     DISABLED     // PD2 - TIM2_CH3
#define CONFIG_PWM1     ENABLED     // PD3 - TIM2_CH2  
#define CONFIG_PWM2     DISABLED     // PD4 - TIM2_CH1
#define CONFIG_PWM3     DISABLED     // PD5 - TIM2_CH4
#define CONFIG_PWM4     DISABLED     // PC0 - TIM2_CH3
#define CONFIG_PWM5     DISABLED     // PC1 - TIM2_CH4

// TIMER1 RELATED PINS WERE ALL REMAPPED (UNDERSCORE "_" indicates remapped)
#define CONFIG_PWM6     ENABLED      // PC7 - T1CH2_ 
#define CONFIG_PWM7     DISABLED     // PC4 - TIM1_CH1
#define CONFIG_PWM8     DISABLED    // PC5 - TIM1_CH3 
#define CONFIG_PWM9     DISABLED    // PD4 - TIM1_CH4

// auxiliary pins
#define CONFIG_PWM10    DISABLED    // PC3 - TIM1_CH1N
#define CONFIG_PWM11    DISABLED    // PD2 - TIM1_CH2N
#define CONFIG_PWM12    DISABLED    // PC6 - TIM1_CH3N

// Default timer frequencies (Hz)
#define PWM_TIM1_FREQUENCY     1000
#define PWM_TIM2_FREQUENCY     60

#define PWM_TIM1_SET ENABLED
#define PWM_TIM2_SET ENABLED

// =============================================================================
// CONFLICT VERIFICATION - PREVENTS COMPILATION IF THERE ARE CONFLICTS
// =============================================================================

// -----------------------------------------------------------------------------
// 1. DIRECT PIN CONFLICTS BETWEEN TIM1 AND TIM2
// -----------------------------------------------------------------------------

// CONFLICT: PD4 used by TIM2_CH1 (PWM2) and TIM1_CH4 (PWM9)
#if (CONFIG_PWM2 == ENABLED) && (CONFIG_PWM9 == ENABLED)
#error "CRITICAL CONFLICT: PD4 is being used by PWM2 (TIM2_CH1) and PWM9 (TIM1_CH4)"
#endif

// CONFLICT: PD2 used by TIM2_CH3 (PWM0) and TIM1_CH2N (PWM11)  
#if (CONFIG_PWM0 == ENABLED) && (CONFIG_PWM11 == ENABLED)
#error "CRITICAL CONFLICT: PD2 is being used by PWM0 (TIM2_CH3) and PWM11 (TIM1_CH2N)"
#endif

// CONFLICT: PC1 used by TIM2_CH4 (PWM5) and TIM1 BKIN (not PWM but same functionality)
#if (CONFIG_PWM5 == ENABLED) && (CONFIG_PWM6 == ENABLED)
#pragma message "WARNING: PC1 (PWM5) and PC7 (PWM6) share GPIO port, check power consumption"
#endif

// -----------------------------------------------------------------------------
// 2. TIMER CONFIGURATION VERIFICATION
// -----------------------------------------------------------------------------

// ERROR: No timer enabled
#if (PWM_TIM1_SET == DISABLED) && (PWM_TIM2_SET == DISABLED)
#error "CONFIGURATION ERROR: No PWM timer enabled. Enable PWM_TIM1_SET or PWM_TIM2_SET"
#endif

// ERROR: TIM1 PWMs enabled but timer disabled
#if ((CONFIG_PWM6 == ENABLED) || (CONFIG_PWM7 == ENABLED) || \
     (CONFIG_PWM8 == ENABLED) || (CONFIG_PWM9 == ENABLED) || \
     (CONFIG_PWM10 == ENABLED) || (CONFIG_PWM11 == ENABLED) || \
     (CONFIG_PWM12 == ENABLED)) && \
     (PWM_TIM1_SET == DISABLED)
#error "CONFIGURATION ERROR: TIM1 PWMs enabled but PWM_TIM1_SET is disabled"
#endif

// ERROR: TIM2 PWMs enabled but timer disabled
#if ((CONFIG_PWM0 == ENABLED) || (CONFIG_PWM1 == ENABLED) || \
     (CONFIG_PWM2 == ENABLED) || (CONFIG_PWM3 == ENABLED) || \
     (CONFIG_PWM4 == ENABLED) || (CONFIG_PWM5 == ENABLED)) && \
     (PWM_TIM2_SET == DISABLED)
#error "CONFIGURATION ERROR: TIM2 PWMs enabled but PWM_TIM2_SET is disabled"
#endif

// -----------------------------------------------------------------------------
// 3. FREQUENCY RANGE VALIDATION
// -----------------------------------------------------------------------------

// ERROR: TIM1 frequency out of range
#if (PWM_TIM1_FREQUENCY < 1) || (PWM_TIM1_FREQUENCY > 100000)
#error "FREQUENCY ERROR: PWM_TIM1_FREQUENCY out of valid range (1-100000 Hz)"
#endif

// ERROR: TIM2 frequency out of range  
#if (PWM_TIM2_FREQUENCY < 1) || (PWM_TIM2_FREQUENCY > 100000)
#error "FREQUENCY ERROR: PWM_TIM2_FREQUENCY out of valid range (1-100000 Hz)"
#endif

// -----------------------------------------------------------------------------
// 4. CONFIGURATION WARNINGS
// -----------------------------------------------------------------------------

// WARNING: PWM6 enabled - requires remapping
#if (CONFIG_PWM6 == ENABLED)
#pragma message "=== PWM6 WARNING ==="
#pragma message "PWM6 (PC7/T1CH2_) enabled:"
#pragma message "- Configure AFIO->PCFR1[7:6] = 11 (Complete mapping)"
#pragma message "- PC7 will be used exclusively by TIM1"
#pragma message "==================="
#endif

// WARNING: Multiple PWMs on same timer channel
#if ((CONFIG_PWM0 == ENABLED) && (CONFIG_PWM4 == ENABLED))
#pragma message "WARNING: PWM0 and PWM4 both use TIM2_CH3 - only one will work at a time"
#endif

#if ((CONFIG_PWM3 == ENABLED) && (CONFIG_PWM5 == ENABLED))  
#pragma message "WARNING: PWM3 and PWM5 both use TIM2_CH4 - only one will work at a time"
#endif

// WARNING: Current configuration summary
#if (CONFIG_PWM6 == ENABLED)
#pragma message "CURRENT CONFIGURATION: TIM1 remapped with PWM6 on PC7"
#endif

#if (CONFIG_PWM1 == ENABLED)
#pragma message "CURRENT CONFIGURATION: TIM2 with PWM1 on PD3"
#endif

// -----------------------------------------------------------------------------
// 5. FINAL SANITY CHECK
// -----------------------------------------------------------------------------

// SUCCESS: Valid configuration
#if (PWM_TIM1_SET == ENABLED) || (PWM_TIM2_SET == ENABLED)
#pragma message "PWM Configuration: VALID - Compilation can proceed"
#endif

#endif /* PWM_PRM_H_ */