
/**************************************************************************//**
 * @file     debounce.c
 * @version  V0.01
 * $Revision: 1 $
 * $Date: 16/06/08 2:29p $
 * @brief    Software debounce API with NUC122 Series.
 *           It uses GPIO interrupt with timer timeout to filter I/O bouncing.
 *           
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC122.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

// <o> Debounce Timer Selection <0=> TIMER0 <1=> TIMER1 <2=> TIMER2 <3=> TIMER3
// <i> To selection Timer which is used to debounce GPIO input.

/******************************************************************
 Modify Here:
    A timer for debounce is necessary. TIMER0 ~ 3 could be configured here.
    
 *******************************************************************/
#define TIMER_NUM       0       /* Select TIMER0, TIMER1, TIMER2 or TIMER3 */

// <o> Debounce Time <1-10000:1><#*10>
// <i> Debounce Timer setting in ms
#define DEBOUNCE_TIME           30                      /* Debounce time in 0.1ms unit */
#define TIMER_COMPARED_VALUE    DEBOUNCE_TIME*1200      /* Timer compared value */


/******************************************************************
 Modify Here:
    GPIO Port and bit could be modify here.
    
 *******************************************************************/
#define PORT                    PC                      /* GPIO Port: PA, PB, PC, PD or PE */
#define BIT                     8                       /* Bit of the GPIO Port. it could be 0 ~ 15 */
#define PIN                     PC8                     /* GPIO Pin bit access pin name */
#define GPIO_IRQHandler         GPCD_IRQHandler
#define GPIO_IRQn               GPCD_IRQn


#if (TIMER_NUM == 0)
# define TIMER                   TIMER0                  
# define TIMER_CLKEN_Msk         CLK_APBCLK_TMR0_EN_Msk
# define TIMER_CLKSRC_Msk        CLK_CLKSEL1_TMR0_S_Msk
# define TIMER_CLKSRC            CLK_CLKSEL1_TMR0_S_HXT
# define DBNCE_IRQHandler        TMR0_IRQHandler
# define TIMER_IRQn              TMR0_IRQn
#elif (TIMER_NUM == 1)
# define TIMER                   TIMER1                  
# define TIMER_CLKEN_Msk         CLK_APBCLK_TMR1_EN_Msk
# define TIMER_CLKSRC_Msk        CLK_CLKSEL1_TMR1_S_Msk
# define TIMER_CLKSRC            CLK_CLKSEL1_TMR1_S_HXT
# define DBNCE_IRQHandler        TMR1_IRQHandler
# define TIMER_IRQn              TMR1_IRQn
#elif (TIMER_NUM == 2)
# define TIMER                   TIMER2                  
# define TIMER_CLKEN_Msk         CLK_APBCLK_TMR2_EN_Msk
# define TIMER_CLKSRC_Msk        CLK_CLKSEL1_TMR2_S_Msk
# define TIMER_CLKSRC            CLK_CLKSEL1_TMR2_S_HXT
# define DBNCE_IRQHandler        TMR2_IRQHandler
# define TIMER_IRQn              TMR2_IRQn
#elif (TIMER_NUM == 3)
# define TIMER                   TIMER3                  
# define TIMER_CLKEN_Msk         CLK_APBCLK_TMR3_EN_Msk
# define TIMER_CLKSRC_Msk        CLK_CLKSEL1_TMR3_S_Msk
# define TIMER_CLKSRC            CLK_CLKSEL1_TMR3_S_HXT
# define DBNCE_IRQHandler        TMR3_IRQHandler
# define TIMER_IRQn              TMR3_IRQn
#endif

volatile uint32_t g_u32Debounce = 0;

void DBNCE_Init(void)
{
    /* Enable Timer Clock Source */
    CLK->APBCLK |= TIMER_CLKEN_Msk;
    
    /* Select Clock as LIRC 10kHz */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & (~TIMER_CLKSRC_Msk)) | TIMER_CLKSRC;
    
    /* Reset Timer */
    TIMER->TCSR = TIMER_TCSR_CRST_Msk;
    while(TIMER->TCSR);
            
    /* Enable Timer IRQ */
    NVIC_EnableIRQ(TIMER_IRQn);
    
    
    /******************************************************************
     Modify Here:
        All Debounce GPIO should be configured here.
          1. Set GPIO to be input mode
          2. Enable GPIO interrupt with rising + falling edge trigger.
          3. Enable GPIO IRQ
     *******************************************************************/
    
    /* Set GPIO Input */
    GPIO_SetMode(PORT, (1<<BIT), GPIO_PMD_INPUT);    
    
    /* Interrupt Type: Both Edge */
    GPIO_EnableInt(PORT, BIT, GPIO_INT_BOTH_EDGE);
    
    /* Enable GPIO IRQ */
    NVIC_EnableIRQ(GPIO_IRQn);    
    
}

void DBNCE_IRQHandler(void)
{
    /******************************************************************
     Modify Here:
       Debounce Ok when timer timeout.
       All GPIO debounce result should be return by global variable here.
     *******************************************************************/
     g_u32Debounce = PIN;
     
    
    /* Clear Timer Interrupt Flag */
    TIMER_ClearIntFlag(TIMER);
}

void GPIO_IRQHandler(void)
{
    /* Reset Timer Counter */
    TIMER->TCSR = TIMER_TCSR_CRST_Msk;
    while(TIMER->TCSR);
    
    /* Debounce Time */
    TIMER_SET_CMP_VALUE(TIMER, TIMER_COMPARED_VALUE-1);
    
    /* Start Timer */
    TIMER->TCSR = TIMER_TCSR_IE_Msk | TIMER_TCSR_CEN_Msk | TIMER_TCSR_TDR_EN_Msk | TIMER_TCSR_CACT_Msk;       
    while(TIMER_IS_ACTIVE(TIMER));

    /*******************************************************************
     Modify Here:
       Clear relative GPIO interrupt flag here
     *******************************************************************/
    GPIO_CLR_INT_FLAG(PORT, (1 << BIT));
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
