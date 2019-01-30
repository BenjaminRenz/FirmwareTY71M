/**************************************************************************//**
 * @file     main.c
 * @version  V2.00
 * $Revision: 3 $
 * $Date: 15/01/09 9:21a $
 * @brief    Capture the PWMA Channel 1 waveform by PWMA Channel 0.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC122.h"

#define PLLCON_SETTING  CLK_PLLCON_60MHz_HXT

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/**
 * @brief       PWMA IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWMA interrupt event
 */
void PWMA_IRQHandler(void)
{
    uint32_t u32CapIntFlag1;

    /* Handle PWMA Capture function */
    u32CapIntFlag1 = PWMA->CCR0;

    /* PWMA channel 0 Capture interrupt */
    if(u32CapIntFlag1 & PWM_CCR0_CAPIF0_Msk)
    {
        PWMA->CCR0 &= (PWM_CCR_MASK | PWM_CCR0_CAPIF0_Msk);
    }
}

/*--------------------------------------------------------------------------------------*/
/* Capture function to calculate the input waveform information                         */
/* u32Count[4] : Keep the internal counter value when input signal rising / falling     */
/*               happens                                                                */
/*                                                                                      */
/* time    A    B     C     D                                                           */
/*           ___   ___   ___   ___   ___   ___   ___   ___                              */
/*      ____|   |_|   |_|   |_|   |_|   |_|   |_|   |_|   |_____                        */
/* index              0 1   2 3                                                         */
/*                                                                                      */
/* The capture internal counter down count from 0x10000, and reload to 0x10000 after    */
/* input signal falling happens (Time B/C/D)                                            */
/*--------------------------------------------------------------------------------------*/
void CalPeriodTime()
{
    uint16_t u32Count[4];
    uint32_t u32i;
    uint16_t u16RisingTime, u16FallingTime, u16HighPeriod, u16LowPeriod, u16TotalPeriod;

    /* Clear Capture Falling Indicator (Time A) */
    PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | PWM_CCR0_CFLRI0_Msk;

    /* Wait for Capture Falling Indicator  */
    while((PWMA->CCR0 >> PWM_CCR0_CFLRI0_Pos & 1) == 0);

    /* Clear Capture Falling Indicator (Time B)*/
    PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | PWM_CCR0_CFLRI0_Msk;

    u32i = 0;

    while(u32i < 4)
    {
        /* Wait for Capture Falling Indicator */
        while((PWMA->CCR0 >> PWM_CCR0_CFLRI0_Pos & 1) == 0);

        /* Clear Capture Falling and Rising Indicator */
        PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | (PWM_CCR0_CFLRI0_Msk | PWM_CCR0_CRLRI0_Msk);

        /* Get Capture Falling Latch Counter Data */
        u32Count[u32i++] = PWMA->CFLR0;

        /* Wait for Capture Rising Indicator */
        while((PWMA->CCR0 >> PWM_CCR0_CRLRI0_Pos & 1) == 0);

        /* Clear Capture Rising Indicator */
        PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | PWM_CCR0_CRLRI0_Msk;

        /* Get Capture Rising Latch Counter Data */
        u32Count[u32i++] = PWMA->CRLR0;
    }

    u16RisingTime = u32Count[1];

    u16FallingTime = u32Count[0];

    u16HighPeriod = u32Count[1] - u32Count[2];

    u16LowPeriod = 0x10000 - u32Count[1];

    u16TotalPeriod = 0x10000 - u32Count[2];

    printf("\nPWM generate: \nHigh Period=7199 ~ 7201, Low Period=16799 ~ 16801, Total Period=23999 ~ 24001\n");
    printf("\ncapture Result: Rising Time = %d, Falling Time = %d \nHigh Period = %d, Low Period = %d, Total Period = %d.\n\n",
           u16RisingTime, u16FallingTime, u16HighPeriod, u16LowPeriod, u16TotalPeriod);
    if((u16HighPeriod < 7199) || (u16HighPeriod > 7201) || (u16LowPeriod < 16799) || (u16LowPeriod > 16801) || (u16TotalPeriod < 23999) || (u16TotalPeriod > 24001))
        printf("Capture Test Fail!!\n");
    else
        printf("Capture Test Pass!!\n");
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184MHz clock */
    CLK->PWRCON |= CLK_PWRCON_OSC22M_EN_Msk;

    /* Waiting for Internal RC clock ready */
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_OSC22M_STB_Msk));

    /* Switch HCLK clock source to Internal RC and and HCLK source divide 1 */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLK_S_Msk) | CLK_CLKSEL0_HCLK_S_HIRC;
    CLK->CLKDIV &= ~CLK_CLKDIV_HCLK_N_Msk;
    CLK->CLKDIV |= (CLK_CLKDIV_HCLK(1) << CLK_CLKDIV_HCLK_N_Pos);

    /* Enable external XTAL 12MHz clock */
    CLK->PWRCON |= CLK_PWRCON_XTL12M_EN_Msk;

    /* Waiting for external XTAL clock ready */
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_XTL12M_STB_Msk));

    /* Set core clock as PLL_CLOCK from PLL */
    CLK->PLLCON = PLLCON_SETTING;
    while(!(CLK->CLKSTATUS & CLK_CLKSTATUS_PLL_STB_Msk));
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLK_S_Msk) | CLK_CLKSEL0_HCLK_S_PLL;

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Enable IP clock */
    CLK->APBCLK = CLK_APBCLK_UART0_EN_Msk | CLK_APBCLK_PWM01_EN_Msk | CLK_APBCLK_PWM23_EN_Msk;

    /* Select UART module clock source */
    CLK->CLKSEL1 &= ~CLK_CLKSEL1_UART_S_Msk;
    CLK->CLKSEL1 |= CLK_CLKSEL1_UART_S_HXT;

    /* select PWM clock source */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~(CLK_CLKSEL1_PWM01_S_Msk | CLK_CLKSEL1_PWM23_S_Msk)) | (CLK_CLKSEL1_PWM01_S_HXT | CLK_CLKSEL1_PWM23_S_HXT);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);

    /* Set GPE multi-function pins for PWMA Channel 0 and channel 1 */
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA12_Msk | SYS_GPA_MFP_PA13_Msk);
    SYS->GPA_MFP |= SYS_GPA_MFP_PA12_PWM0 | SYS_GPA_MFP_PA13_PWM1;

}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART IP */
    SYS->IPRSTC2 |=  SYS_IPRSTC2_UART0_RST_Msk;
    SYS->IPRSTC2 &= ~SYS_IPRSTC2_UART0_RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 115200);
    UART0->LCR = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    printf("+------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  This sample code will use PWMA channel 0 to capture\n  the signal from PWMA channel 1.\n");
    printf("  I/O configuration:\n");
    printf("    PWM0(PA.12 PWMA channel 0) <--> PWM1(PA.13 PWMA channel 1)\n\n");
    printf("Use PWMA Channel 0(PA.12) to capture the PWMA Channel 1(PA.13) Waveform\n");

    while(1)
    {
        printf("Press any key to start PWM Capture Test\n");
        getchar();

        /*--------------------------------------------------------------------------------------*/
        /* Set the PWMA Channel 1 as PWM output function.                                               */
        /*--------------------------------------------------------------------------------------*/

        /* Assume PWM output frequency is 250Hz and duty ratio is 30%, user can calculate PWM settings by follows.
           duty ratio = (CMR+1)/(CNR+1)
           cycle time = CNR+1
           High level = CMR+1
           PWM clock source frequency = __HXT = 12000000
           (CNR+1) = PWM clock source frequency/prescaler/clock source divider/PWM output frequency
                   = 12000000/2/1/250 = 24000
           (Note: CNR is 16 bits, so if calculated value is larger than 65536, user should increase prescale value.)
           CNR = 23999
           duty ratio = 30% ==> (CMR+1)/(CNR+1) = 30%
           CMR = 7199
           Prescale value is 1 : prescaler= 2
           Clock divider is PWM_CSR_DIV1 : clock divider =1
        */
        /*Set Pwm mode*/
        PWMA->PCR |= PWM_PCR_CH1MOD_Msk;

        /*Set PWM Timer clock prescaler*/
        PWM_SET_PRESCALER(PWMA, PWM_CH1, 1); // Divided by 2

        /*Set PWM Timer clock divider select*/
        PWM_SET_DIVIDER(PWMA, PWM_CH1, PWM_CLK_DIV_1);

        /*Set PWM Timer duty*/
        PWM_SET_CMR(PWMA, PWM_CH1, 7199);

        /*Set PWM Timer period*/
        PWM_SET_CNR(PWMA, PWM_CH1, 23999);

        /* Enable PWM Output path for PWMA channel 1 */
        PWMA->POE |= PWM_POE_PWM1_Msk;

        /* Enable Timer for PWMA channel 1 */
        PWMA->PCR |= PWM_PCR_CH1EN_Msk;

        /*--------------------------------------------------------------------------------------*/
        /* Set the PWMA channel 0 for capture function                                         */
        /*--------------------------------------------------------------------------------------*/

        /* If input minimum frequency is 250Hz, user can calculate capture settings by follows.
           Capture clock source frequency = __HXT = 12000000 in the sample code.
           (CNR+1) = Capture clock source frequency/prescaler/clock source divider/minimum input frequency
                   = 12000000/2/1/250 = 24000
           (Note: CNR is 16 bits, so if calculated value is larger than 65536, user should increase prescale value.)
           CNR = 0xFFFF
           (Note: In capture mode, user should set CNR to 0xFFFF to increase capture frequency range.)
        */
        /*Set Pwm mode*/
        PWMA->PCR |= PWM_PCR_CH0MOD_Msk;

        /*Set PWM Timer clock prescaler*/
        PWM_SET_PRESCALER(PWMA, PWM_CH0, 1); // Divided by 2

        /*Set PWM Timer clock divider select*/
        PWM_SET_DIVIDER(PWMA, PWM_CH0, PWM_CLK_DIV_1);

        /*Set PWM Timer loaded value*/
        PWM_SET_CNR(PWMA, PWM_CH0, 0xFFFF);

        /* Enable capture falling edge interrupt for PWMA channel 0 */
        PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | PWM_CCR0_CFL_IE0_Msk;

        /* Enable PWMA NVIC interrupt */
        NVIC_EnableIRQ((IRQn_Type)(PWMA_IRQn));

        /* Enable Capture Function for PWMA channel 0 */
        PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | PWM_CCR0_CAPCH0EN_Msk;

        /* Enable Timer for PWMA channel 0 */
        PWMA->PCR |= PWM_PCR_CH0EN_Msk;

        /* Wait until PWMA channel 0 Timer start to count */
        while(PWMA->PDR0 == 0);

        /* Enable capture input path for PWMA channel 0 */
        PWMA->CAPENR |= 0x1;

        /* Capture the Input Waveform Data */
        CalPeriodTime();
        /*------------------------------------------------------------------------------------------------------*/
        /* Stop PWMA channel 1 (Recommended procedure method 1)                                                 */
        /* Set PWM Timer loaded value(CNR) as 0. When PWM internal counter(PDR) reaches to 0, disable PWM Timer */
        /*------------------------------------------------------------------------------------------------------*/

        /* Set PWMA channel 1 loaded value as 0 */
        PWMA->CNR1 = 0;

        /* Wait until PWMA channel 1 Timer Stop */
        while(PWMA->PDR1 != 0);

        /* Disable Timer for PWMA channel 1 */
        PWMA->PCR &= ~PWM_PCR_CH1EN_Msk;

        /* Disable PWM Output path for PWMA channel 1 */
        PWMA->POE &= ~PWM_POE_PWM1_Msk;

        /*------------------------------------------------------------------------------------------------------*/
        /* Stop PWMA channel 0 (Recommended procedure method 1)                                                 */
        /* Set PWM Timer loaded value(CNR) as 0. When PWM internal counter(PDR) reaches to 0, disable PWM Timer */
        /*------------------------------------------------------------------------------------------------------*/

        /* Disable PWMA NVIC */
        NVIC_DisableIRQ((IRQn_Type)(PWMA_IRQn));

        /* Set loaded value as 0 for PWMA channel 0 */
        PWMA->CNR0 = 0;

        /* Wait until PWMA channel 0 current counter reach to 0 */
        while(PWMA->PDR0 != 0);

        /* Disable Timer for PWMA channel 0 */
        PWMA->PCR &= ~PWM_PCR_CH0EN_Msk;

        /* Disable Capture Function for  PWMA channel 0*/
        PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) & ~PWM_CCR0_CAPCH0EN_Msk;

        /* Clear Capture Interrupt flag for PWMA channel 0*/
        PWMA->CCR0 = (PWMA->CCR0 & PWM_CCR_MASK) | (PWM_CCR0_CAPIF0_Msk);

        /* Disable Capture Input path for PWMA channel 0 */
        PWMA->CAPENR &= ~0x1;
    }
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
