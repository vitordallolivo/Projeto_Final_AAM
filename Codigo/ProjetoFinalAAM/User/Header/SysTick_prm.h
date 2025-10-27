#ifndef SYSTICK_PRM_H
#define SYSTICK_PRM_H

// =============================================================================
// CONFIGURA??O DO SYSTICK
// =============================================================================

// Frequ¨ºncia do SysTick em Hz (1000 = 1ms, 100 = 10ms, etc)
#define SYSTICK_FREQUENCY_HZ         10000 // 100us


// Prioridade da interrupcao do SysTick (0 = m¨¢xima prioridade)
#define SYSTICK_PRIORITY             0

// Habilita/desabilita funcoes de debug
#define SYSTICK_DEBUG_ENABLED        1

//------------------------- Defines, Enumerations --------------------------------
typedef enum
{
    //Add MsTimers here
    TIMER_MS_PWM = 0,
    TIMER_MS_LOADCELL,
    TIMER_MS_RPM,         
    TIMER_MS_SOUND,
    TIMER_MS_COMMUNICATION,
    // Do NOT REMOVE the line below
    NUM_OF_MS_TIMERS			// Total timers installed -- DON?T remove it
} MS_TIMER_NAME;


/**
 * TIMERS RESOLUTION
 *
 * Recommended values in ms: 1, 5, 20, 25, 50, 100, or 1000
 */
#define MS_TIMERS_RESOLUTION      10 //1ms , para 10khz -> 1 ms s?o 10 ticks

#define HMS_TIMERS_RESOLUTION     50//5

#endif