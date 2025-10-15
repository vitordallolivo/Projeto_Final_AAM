#ifndef SYSTICK_H
#define SYSTICK_H

#include "ch32v00x.h"
#include "SysTick_prm.h"

// =============================================================================
// DEFINI??ES CORRETAS BASEADAS NO MANUAL
// =============================================================================

// SysTick Control Register (STK_CTLR) bits - CORRIGIDOS
#define SYSTICK_CTLR_STE      (1 << 0)    // SysTick Enable
#define SYSTICK_CTLR_STIE     (1 << 1)    // SysTick Interrupt Enable  
#define SYSTICK_CTLR_STCLK    (1 << 2)    // Clock source selection (0=HCLK/8, 1=HCLK)
#define SYSTICK_CTLR_STRE     (1 << 3)    // Auto-reload enable
#define SYSTICK_CTLR_CNTIF    (1 << 4)    // Count direction (0=up, 1=down)
#define SYSTICK_CTLR_SWIE     (1 << 31)   // Software interrupt trigger

// SysTick Status Register (STK_SR) bits
#define SYSTICK_SR_CNTIF      (1 << 0)    // Count flag

// Estados do SysTick
typedef enum {
    SYSTICK_DISABLED = 0,
    SYSTICK_RUNNING
} SYSTICK_STATE_TYPE;

// Callback function type
typedef void (*SysTick_Callback_t)(void);

// =============================================================================
// -------------------VARIAVEIS PUBLICAS--------------------------------------
// =============================================================================

extern volatile uint32_t SysTick_Count;
extern volatile SYSTICK_STATE_TYPE SysTick_State;

// =============================================================================
// PROTOTIPOS DAS FUNCOES PUBLICAS
// =============================================================================

void SysTick_Init(void);
void SysTick_DeInit(void);
uint32_t SysTick_GetTick(void);
void SysTick_Delay(uint32_t ms);
uint32_t SysTick_GetElapsedTime(uint32_t previous_tick);
void SysTick_SetCallback(SysTick_Callback_t callback);
void SysTick_Handler(void);

#endif