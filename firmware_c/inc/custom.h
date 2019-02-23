
#ifndef CUSTOM_H_INCLUDED
#define CUSTOM_H_INCLUDED

#include <stdint.h>
//variable names are derived from the manual at http://www.nuvoton.com/resource-files/TRM_NUC123_Series_EN_Rev2.04.pdf

//Clock configuration defines
#define AHB_BASE 0x50000000
#define CLK_BASE_POINTER ((uint32_t*)(AHB_BASE+0x00200))
#define REGWRPROT (*((uint32_t*)(AHB_BASE+0x100)))

#define CLK_AHBCLK_ISP_Msk              0x00000004
#define CLK_APBCLK_USBD_Msk             0x08000000
#define CLK_APBCLK_I2C1_Msk             0x00000200
#define CLK_APBCLK_UART0_Msk            0x00010000

#define CLK_CLKSEL1_WDT_S_HCLK_DIV2048  0x00000002
#define CLK_CLKSEL1_UART_S_PLL          0x01000000

#define CLK_PWRCON_XTL12M_EN_Msk        0x00000001
#define CLK_PWRCON_OSC22M_EN_Msk        0x00000004
#define CLK_PWRCON_PD_WU_INT_EN_Msk     0x00000020
#define CLK_PWRCON_PD_WU_DLY_Msk        0x00000010

#define CLK_CLKSTATUS_OSC22M_STB_Msk    0x00000010
#define CLK_CLKSTATUS_PLL_STB_Msk       0x00000004
#define CLK_CLKSTATUS_XTL12M_STB_Msk    0x00000001

#define CLK_CLKSEL0_HCLK_S_PLL          0x00000002
#define CLK_CLKSEL0_STCLK_S_HCLK_DIV2   0x00000018

#define CLK_CLKDIV_USB(c)  (((c)-1) << 4)
#define CLK_CLKDIV_HCLK(c)  ((c)-1)

//GPIO defines
#define GP_BA 0x50004000 //memory base address of the GPIO controller

#define GPIOA_PMD (*((uint32_t*)(GP_BA+0x000))) //pin mode (input/output/tristate/bidirectional) configuration
#define GPIOB_PMD (*((uint32_t*)(GP_BA+0x040)))
#define GPIOC_PMD (*((uint32_t*)(GP_BA+0x080)))
#define GPIOD_PMD (*((uint32_t*)(GP_BA+0x0C0)))
#define GPIOF_PMD (*((uint32_t*)(GP_BA+0x140)))


#define GPIOA12_DOUT (*((uint32_t*)(GP_BA+0x230))) //high backlight on, low backlight off
#define GPIOB10_DOUT (*((uint32_t*)(GP_BA+0x268))) //clock to shift through the rgister
#define GPIOC0_DOUT  (*((uint32_t*)(GP_BA+0x280)))//pull low then high to display internal shifted values on the output pins
#define GPIOC1_DOUT  (*((uint32_t*)(GP_BA+0x284)))//connected to data in pin on first shift register

//System Manager defines
#define GCR_BA (0x50000000)
#define GPA_MFP (*((uint32_t*)(GCR_BA+0x30)))  //GPIO A Input Type Register
#define GPB_MFP (*((uint32_t*)(GCR_BA+0x34)))  //GPIO B Input Type Register
#define ALT_MFP (*((uint32_t*)(GCR_BA+0x50)))  //GPIO Alternative Functions
#define ALT_MFP1 (*((uint32_t*)(GCR_BA+0x54))) //GPIO Alternative Functions

static __inline void  SYS_UnlockReg(void){
    while(REGWRPROT != 0x00000001){ //Check if unlocked
        REGWRPROT = 0x59;           //Write unlock sequence
        REGWRPROT = 0x16;
        REGWRPROT = 0x88;
    }
}

static __inline void SYS_LockReg(void){ //Relock Register by writing arbitrary data to it
    REGWRPROT = 0;
}

typedef struct{
    uint32_t PWRCON;
    uint32_t AHBCLK;
    uint32_t APBCLK;
    uint32_t CLKSTATUS;
    uint32_t CLKSEL0;
    uint32_t CLKSEL1;
    uint32_t CLKDIV;
    uint32_t CLKSEL2;
    uint32_t PLLCON;
} CLK_T;

//Interrupt type
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
//number of interrupt priority bits
#define __NVIC_PRIO_BITS 2
//Include of standard arm headers
#include "core_cm0.h"     //Core periphial library header
#include "core_cmInstr.h" //Assembler Instructions

#endif // CUSTOM_H_INCLUDED
