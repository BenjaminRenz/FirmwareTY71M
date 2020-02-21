#ifndef NVIC_H_INCLUDED
#define NVIC_H_INCLUDED

typedef enum IRQn{
    //Cortex-m0
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn      = -13,
    SVCall_IRQn         = -5,
    PendSV_IRQn         = -2,
    SysTick_IRQn        = -1,
    //ARMIKMCU
    BOD_IRQn            = 0,
    WDT_IRQn            = 1,
    EINT0_IRQn          = 2,
    EINT1_IRQn          = 3,
    GPAB_IRQn           = 4,
    GPCD_IRQn           = 5,
    PWMA_IRQn           = 6,
    TMR0_IRQn           = 8,
    TMR1_IRQn           = 9,
    TMR2_IRQn           = 10,
    TMR3_IRQn           = 11,
    UART0_IRQn          = 12,
    UART1_IRQn          = 13,
    SPI0_IRQn           = 14,
    SPI1_IRQn           = 15,
    I2C1_IRQn           = 19,
    USBD_IRQn           = 23,
    PS2_IRQn            = 24,
    PWRWU_IRQn          = 28,
    RTC_IRQn            = 31
} IRQn_Type;
#define __NVIC_PRIO_BITS 2
#include "core_cm0.h" //After typedef IRQn_Type


#define SCS_BA 0xE000E000
#define INT_BA 0x50000300
#define NVIC_ISER (*((uint32_t*)(SCS_BA+0x100))) //interrupt set enable for all IRQ's
#define NVIC_ICER (*((uint32_t*)(SCS_BA+0x180))) //interrupt clear enable for all IRQ's
/*
    IRQ12 is UART0 interrupt
    IRQ19 is I2C1 interrupt
    IRQ23 is USB interrupt
    IRQ26 is DMA interrupt
*/
void NVIC_init(){
    NVIC_SetPriority(USBD_IRQn,0); //set priority for USB Interrupt to highest priority 0
    NVIC_EnableIRQ(USBD_IRQn); //enable USB interrupt
    NVIC_SetPriority(TMR0_IRQn,3);
    NVIC_EnableIRQ(TMR0_IRQn);
    NVIC_SetPriority(TMR1_IRQn,2);  //TODO change? priority for key report
    NVIC_EnableIRQ(TMR1_IRQn);
    NVIC_SetPriority(UART0_IRQn,1);
    NVIC_EnableIRQ(UART0_IRQn);
}


#endif // NVIC_H_INCLUDED
