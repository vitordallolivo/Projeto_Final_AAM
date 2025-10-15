#include "../Header/SysTick.h"

// =============================================================================
// -----------------------VARIAVEIS GLOBAIS------------------------------------
// =============================================================================

volatile uint32_t SysTick_Count = 0;
volatile SYSTICK_STATE_TYPE SysTick_State = SYSTICK_DISABLED;
static SysTick_Callback_t User_Callback = NULL;

// =============================================================================
// ------------------------FUNCOES PUBLICAS-------------------------------------
// =============================================================================

/*******************************************************************************
 * @brief  Inicializa o SysTick com frequ¨ºncia espec¨ªfica
 * @param  frequency: Frequ¨ºncia em Hz (ex: 1000 para 1ms)
 * @retval None
 ******************************************************************************/
void SysTick_Init()
{
    // Desabilita SysTick
    SysTick->CTLR = 0;
    
    // Calcula o valor de comparacao (CMP)
    // Usando HCLK/8 como clock source (STCLK=0)
    uint32_t clock_freq = SystemCoreClock / 8;
    uint32_t cmp_value = (clock_freq / SYSTICK_FREQUENCY_HZ) - 1;
    
    // Verifica se o valor cabe no registrador de 24 bits
    if (cmp_value > 0xFFFFFF) {
        cmp_value = 0xFFFFFF; 
    }
    
    // Configura registradores do SysTick
    SysTick->CNT = 0;           // Reseta contador
    SysTick->CMP = cmp_value;   // Define valor de comparacao
    
    // Configura Control Register:
    // STE=1 (Enable), STIE=1 (Interrupt Enable), STCLK=0 (HCLK/8), STRE=1 (Auto-reload)
    SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STRE;
    
    // Habilita interrupcao global do SysTick no NVIC
    NVIC_EnableIRQ(SysTicK_IRQn);
    
    SysTick_State = SYSTICK_RUNNING;
    SysTick_Count = 0;
}

/*******************************************************************************
 * @brief  Inicializa o SysTick usando HCLK direto (mais preciso)
 * @param  frequency: Frequ¨ºncia em Hz
 * @retval None
 ******************************************************************************/
void SysTick_Init_HCLK(uint32_t frequency)
{
    // Desabilita SysTick
    SysTick->CTLR = 0;
    
    // Calcula o valor de compara??o usando HCLK direto
    uint32_t cmp_value = (SystemCoreClock / frequency) - 1;
    
    if (cmp_value > 0xFFFFFF) {
        cmp_value = 0xFFFFFF;
    }
    
    // Configura registradores
    SysTick->CNT = 0;
    SysTick->CMP = cmp_value;
    
    // Usa HCLK direto (STCLK=1) + outras configura??es
    SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | 
                   SYSTICK_CTLR_STCLK | SYSTICK_CTLR_STRE;
    
    NVIC_EnableIRQ(SysTicK_IRQn);
    
    SysTick_State = SYSTICK_RUNNING;
    SysTick_Count = 0;
}

/*******************************************************************************
 * @brief  Desabilita o SysTick
 * @param  None
 * @retval None
 ******************************************************************************/
void SysTick_DeInit(void)
{
    // Desabilita SysTick
    SysTick->CTLR = 0;
    
    // Desabilita interrupcao no NVIC
    NVIC_DisableIRQ(SysTicK_IRQn);
    
    SysTick_State = SYSTICK_DISABLED;
    SysTick_Count = 0;
    User_Callback = NULL;
}

/*******************************************************************************
 * @brief  Obt¨¦m o contador atual do SysTick
 * @param  None
 * @retval Contador atual em ticks
 ******************************************************************************/
uint32_t SysTick_GetTick(void)
{
    return SysTick_Count;
}

/*******************************************************************************
 * @brief  Delay bloqueante em milissegundos
 * @param  ms: Tempo em milissegundos
 * @retval None
 ******************************************************************************/
void SysTick_Delay(uint32_t ms)
{
    if (SysTick_State != SYSTICK_RUNNING) {
        return;
    }
    
    uint32_t start_tick = SysTick_Count;
    
    // Espera at¨¦ que o tempo passe (tratando overflow)
    while (SysTick_GetElapsedTime(start_tick) < ms) {
        // Wait For Interrupt - economiza energia
        __asm__ volatile ("wfi");
    }
}

/*******************************************************************************
 * @brief  Calcula o tempo decorrido desde um tick anterior
 * @param  previous_tick: Tick de referencia
 * @retval Tempo decorrido em ms
 ******************************************************************************/
uint32_t SysTick_GetElapsedTime(uint32_t previous_tick)
{
    if (SysTick_State != SYSTICK_RUNNING) {
        return 0;
    }
    
    // Lida com overflow do contador (32 bits)
    return (SysTick_Count - previous_tick);
}

/*******************************************************************************
 * @brief  Define uma funcao callback para ser chamada a cada tick
 * @param  callback: Ponteiro para a funcao callback
 * @retval None
 ******************************************************************************/
void SysTick_SetCallback(SysTick_Callback_t callback)
{
    User_Callback = callback;
}

/*******************************************************************************
 * @brief  Handler de interrupcao do SysTick
 * @param  None
 * @retval None
 ******************************************************************************/
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    // Limpa flag de interrupcao (escreve 0 no bit CNTIF)
    SysTick->SR = 0;
    
    // Incrementa contador global
    SysTick_Count++;
    
    if (User_Callback != NULL) {
        User_Callback();
    }
}

/*******************************************************************************
 * @brief  Obt¨¦m o valor atual do contador do SysTick
 * @param  None
 * @retval Valor atual do contador de hardware
 ******************************************************************************/
uint32_t SysTick_GetCurrentValue(void)
{
    return SysTick->CNT;
}

/*******************************************************************************
 * @brief  Define manualmente o valor do contador
 * @param  value: Valor a ser definido
 * @retval None
 ******************************************************************************/
void SysTick_SetCurrentValue(uint32_t value)
{
    SysTick->CNT = value & 0xFFFFFF; // Apenas 24 bits
}