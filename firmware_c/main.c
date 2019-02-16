#define __NO_SYSTEM_INIT
#include "custom.h" //NUC123LD4AN hardware definitions

#include "backlight.h" //fuctions for setting the rgb led backlight
//#include <NUC122.h>
//#include <semihosting.h>
//microcontroller is a NUC123LD4AN with 68kb flash and 20kb sram
//goto pdf reference: http://www.nuvoton.com/resource-files/TRM_NUC123_Series_EN_Rev2.04.pdf

//Note USB supports up to 8 endpoints

void SystemInit(){
    //section: Clock configuration
    /*
        External Quartz (HXT) is 4MHz
        Internal High speed (HIRC) is 22.1MHz +/-3%
        USB needs 48MHz
        Max cortex m0 core speed is 72MHz
        PLL max is 144MHz
    */
    /*
        FIN=4MHz
        Set IN_DV=0       -> NR=2  -> can't change with 4MHz HXT because of Fref restriction
        Set OUT_DV=0      -> NO=1
        Set FB_DV=70      -> NF=72

        Constraints:
        -4MHz<FIN<24MHz                             -> checked
        -0.8MHz<Fref<8MHz                           -> checked
        -100MHz<FCO<200MHz better 120MHz<FCO<200MHz -> checked

        Fref=FIN/2/NR=4MHz/2/2=1MHz
        FCO=Fref*2*NF=1MHz*2*72=144MHz
        FOUT=FIN*NF/NR/NO=4MHz*96/2/1=144MHz

        Dividers
        USB: /3 by CLKDIV     -> 48MHz
        HCLK_N: /2 by CLKDIV  -> 72MHz
            used to be CLKSEL0_HCLS_S_PLL_DIV2 but conflicting definitions in the manual/library

        CLOCK INIT PROCESS:
        write all other registers, *1 to enable HXT, wait for stable clock, *2 to switch to HXT, *3 to disable HIRC

        This leads to register config:
        PWRCON *1:              0x00000035 //Instant powerdown with PWR_DOWN_EN, Interrupts enables, wait for stable clock, HIRC on, HXT on
        PWRCON *3:              0x00000031  //Instant powerdown with PWR_DOWN_EN, Interrupts enables, wait for stable clock, HXT on
        AHBCLK:                 0x00000004 //ISP
        APBCLK:                 0x08010200 //USBD, UART0, I2C1
        CLKSEL0 *2:             0x00000001 //STCLK_S=HXT/1,HCLK_S=PLL/2 !*only write to after source and destination clock is stable and after
        CLKSEL1:                0x5122227A //all periphial clocks on HCLK
        CLKSEL2:                0x0002000A //all periphial clocks on HCLK
        CLKDIV:                 0x00000020 //USB/3
        PLLCON:                 0x00000046 //Setup for 144MHz see above
        FRQDIV:                 0x00000000 //Frequency divider and clock output pin disabled
        APBDIV:                 0x00000000 //default, no divider
    */
    //Unlock configuration registers
    SYS_UnlockReg();
    //Configure clock for periphials, pll, ...
    CLK_T* ClockContrMemoryMap=(CLK_T*) CLK_BASE;
    ClockContrMemoryMap->AHBCLK=CLK_AHBCLK_ISP_Msk;
    ClockContrMemoryMap->APBCLK=CLK_APBCLK_USBD_Msk|CLK_APBCLK_I2C1_Msk|CLK_APBCLK_UART0_Msk;
    ClockContrMemoryMap->CLKDIV=CLK_CLKDIV_USB(3)|CLK_CLKDIV_HCLK(2);
    ClockContrMemoryMap->PLLCON=70; //FB_DV=70, OUT_DV=0, IN_DV=0, PLL_SRC=HXT
    ClockContrMemoryMap->CLKSEL1=CLK_CLKSEL1_WDT_S_HCLK_DIV2048|CLK_CLKSEL1_UART_S_PLL;
    //ClockContrMemoryMap->CLKSEL2=//CLK_CLKSEL2
    //Enable external HXT
    ClockContrMemoryMap->PWRCON=CLK_PWRCON_XTL12M_EN_Msk|CLK_PWRCON_OSC22M_EN_Msk|CLK_PWRCON_PD_WU_DLY_Msk|CLK_PWRCON_PD_WU_INT_EN_Msk;
    //Wait for stability of external and internal oscillator
    while(((~ClockContrMemoryMap->CLKSTATUS)&(CLK_CLKSTATUS_OSC22M_STB_Msk|CLK_CLKSTATUS_XTL12M_STB_Msk|CLK_CLKSTATUS_PLL_STB_Msk))==0){}
    //Set clock to external
    ClockContrMemoryMap->CLKSEL0=CLK_CLKSEL0_STCLK_S_HCLK_DIV2|CLK_CLKSEL0_HCLK_S_PLL; //Because of conflicting documentation of CLK_CLKSEL0_HCLK_S_LXT I will use CLK_CLKSEL_HCLK_S_PLL, reference says this is PLL/2 but programming library says otherwise
    //DODO use divider to half HCLK
    //Relock configuration registers
    SYS_LockReg();

    //PIN CONFIGURATION

    //Special Pin config
    //I2C1 - SCL PA11, SDA PA10
    //UART0 - RXD PC4, TXD PC5
    GPA_MFP=0x00000C00;
    GPB_MFP=0x00000002;
    ALT_MFP=0x60000000;

    //GPIO Config
    //Switch INPUT  PB8, PF2, PD0-PD5, PA13
    //Switch OUTPUT
    //Battery/USB Voltage detection? PB14/INT0 and PA12




}
int main(void){


}
