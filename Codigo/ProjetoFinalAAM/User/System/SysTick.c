#include "../Header/SysTick.h"

// =============================================================================
// -----------------------VARIAVEIS GLOBAIS------------------------------------
// =============================================================================
unsigned char Flag_tickMS = 0;
volatile uint32_t SysTick_Count = 0;
volatile SYSTICK_STATE_TYPE SysTick_State = SYSTICK_DISABLED;

typedef struct
{
	TIMER_STATUS Status;
    unsigned short int Count;
}TYPE_MSTIMER;

TYPE_MSTIMER Ms_Timers[NUM_OF_MS_TIMERS];

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

     for(uint8_t i = 0; i < NUM_OF_MS_TIMERS; i++) {
        Ms_Timers[i].Status = TIMER_EXPIRED;
        Ms_Timers[i].Count = 0;
    }
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
    for(uint8_t i = 0; i < NUM_OF_MS_TIMERS; i++) {
        Ms_Timers[i].Status = TIMER_EXPIRED;
        Ms_Timers[i].Count = 0;
    }
}

//===================================== MillisecondTimers ==========================+=====================
/**
 *
 */


void Timer__MsHandler(void)
{
    
    if(Flag_tickMS == MS_TIMERS_RESOLUTION){
        unsigned char i;                                	

        for(i=0; i < NUM_OF_MS_TIMERS; i++)                       
        {
            if(Ms_Timers[i].Status == TIMER_IS_RUNNING) 	//Is this timer running?
            {
                Ms_Timers[i].Count--;                   
                if(!(Ms_Timers[i].Count))               
                {
                    Ms_Timers[i].Status = TIMER_EXPIRED;   //Set timer timeout
                }
            }
        }

        Flag_tickMS =0;
    }
}
 
void Timer__MsSet(MS_TIMER_NAME id, unsigned short int ms_value)
{
	unsigned long time;                           

	if (id < NUM_OF_MS_TIMERS)
	{
		if (ms_value < MS_TIMERS_RESOLUTION)                     
		{
			Ms_Timers[id].Status = TIMER_EXPIRED;                   
		}
		else                                        
		{
			Ms_Timers[id].Status = TIMER_EXPIRED;      
			time = ms_value / MS_TIMERS_RESOLUTION;
			Ms_Timers[id].Count = (unsigned short int)time;  
			if (Ms_Timers[id].Count != 0)
			{
				Ms_Timers[id].Status = TIMER_IS_RUNNING;		
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------
/**
 * 
 */
void Timer__MsExpire(MS_TIMER_NAME id)
{
	if (id < NUM_OF_MS_TIMERS)
	{
		Ms_Timers[id].Count = 0;
		Ms_Timers[id].Status = TIMER_EXPIRED;
	}
}

//-----------------------------------------------------------------------------------------------------------------
/**
 * 
 */
TIMER_STATUS Timer__MsGetStatus (MS_TIMER_NAME id)
{
	return (Ms_Timers[id].Status);
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
    Flag_tickMS++;
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
