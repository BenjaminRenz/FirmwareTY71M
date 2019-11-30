
#ifndef CUSTOM_H_INCLUDED
#define CUSTOM_H_INCLUDED

#include <stdint.h>
/*variable names are derived from the manual at
http://www.nuvoton.com/resource-files/TRM_NUC123_Series_EN_Rev2.04.pdf
*/
//Clock configuration defines
#define AHB_BASE 0x50000000
#define CLK_BASE_POINTER ((uint32_t*)(AHB_BASE+0x00200))
#define REGWRPROT (*((uint32_t*)(AHB_BASE+0x100)))

#define CLK_AHBCLK_ISP_Msk              0x00000004
#define CLK_APBCLK_USBD_Msk             0x08000000
#define CLK_APBCLK_I2C1_Msk             0x00000200
#define CLK_APBCLK_UART0_Msk            0x00010000
#define CLK_APBCLK_TMR_all_Msk          0x0000003c

#define CLK_CLKSEL1_WDT_S_HCLK_DIV2048  0x00000002
#define CLK_CLKSEL1_UART_S_PLL          0x01000000
#define CLK_CLKSEL1_TMR0_S_HCLK         0x00000200
#define CLK_CLKSEL1_TMR1_S_HCLK         0x00002000
#define CLK_CLKSEL1_TMR2_S_HCLK         0x00020000
#define CLK_CLKSEL1_TMR3_S_HCLK         0x00200000

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

#define GPIOA_PIN (*((uint32_t*)(GP_BA+0x010))) //input registers
#define GPIOB_PIN (*((uint32_t*)(GP_BA+0x050)))
#define GPIOC_PIN (*((uint32_t*)(GP_BA+0x090)))
#define GPIOD_PIN (*((uint32_t*)(GP_BA+0x0D0)))
#define GPIOF_PIN (*((uint32_t*)(GP_BA+0x150)))

//backlight
#define GPIOA12_DOUT (*((uint32_t*)(GP_BA+0x230))) //high backlight on, low backlight off
#define GPIOB10_DOUT (*((uint32_t*)(GP_BA+0x268))) //clock to shift through the register
#define GPIOC0_DOUT  (*((uint32_t*)(GP_BA+0x280))) //pull low then high to display internal shifted values on the output pins
#define GPIOC1_DOUT  (*((uint32_t*)(GP_BA+0x284))) //connected to data in pin on first shift register

//keymatrix
//out PA13 PD0 PD1 - PD5 PF2 PB8
#define GPIOA13_DOUT_POINTER ((uint32_t*)(GP_BA+0x234))
#define GPIOD0_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2C0))
#define GPIOD1_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2C4))
#define GPIOD2_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2C8))
#define GPIOD3_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2CC))
#define GPIOD4_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2D0))
#define GPIOD5_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2D4))
#define GPIOF2_DOUT_POINTER  ((uint32_t*)(GP_BA+0x348))
#define GPIOB8_DOUT_POINTER  ((uint32_t*)(GP_BA+0x260))
//in
#define GPIOB9_DOUT_POINTER  ((uint32_t*)(GP_BA+0x264))
#define GPIOC13_DOUT_POINTER ((uint32_t*)(GP_BA+0x2B4))
#define GPIOC12_DOUT_POINTER ((uint32_t*)(GP_BA+0x2B0))
#define GPIOC11_DOUT_POINTER ((uint32_t*)(GP_BA+0x2AC))
#define GPIOC10_DOUT_POINTER ((uint32_t*)(GP_BA+0x2A8))
#define GPIOC9_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2A4))
#define GPIOC8_DOUT_POINTER  ((uint32_t*)(GP_BA+0x2A0))
#define GPIOA15_DOUT_POINTER ((uint32_t*)(GP_BA+0x23C))

//System Manager defines
#define GCR_BA (0x50000000)
#define GPA_MFP (*((uint32_t*)(GCR_BA+0x30)))  //GPIO A Alternative Func Register
#define GPB_MFP (*((uint32_t*)(GCR_BA+0x34)))  //GPIO B Alternative Func Register
#define GPC_MFP (*((uint32_t*)(GCR_BA+0x38)))  //GPIO C Alternative Func Register
#define GPD_MFP (*((uint32_t*)(GCR_BA+0x3C)))  //GPIO D Alternative Func Register
#define GPF_MFP (*((uint32_t*)(GCR_BA+0x44)))  //GPIO F Alternative Func Register

#define ALT_MFP (*((uint32_t*)(GCR_BA+0x50)))  //GPIO Alternative Functions
#define ALT_MFP1 (*((uint32_t*)(GCR_BA+0x54))) //GPIO Alternative Functions

#define IPRSTC1 (*((uint32_t*)(GCR_BA+0x08))) //Peripheral Reset Control 2
#define IPRSTC2 (*((uint32_t*)(GCR_BA+0x0C))) //Peripheral Reset Control 2


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

static __inline void I2C1_start_reset(){
    IPRSTC2|=(1<<9);
}
static __inline void I2C1_end_reset(){
    IPRSTC2&=~(1<<9);
}

static __inline void UART0_start_reset(){
    IPRSTC2|=(1<<16);
}
static __inline void UART0_end_reset(){
    IPRSTC2&=~(1<<16);
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

#endif // CUSTOM_H_INCLUDED
