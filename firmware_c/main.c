#define __NO_SYSTEM_INIT
#include "custom.h" //NUC123LD4AN hardware definitions

#include "backlight.h" //functions for setting the rgb led backlight
#include "keymatrix.h" //functions for scanning the key matrix
//#include <NUC122.h>
//#include <semihosting.h>
//microcontroller is a NUC123LD4AN with 68kb flash and 20kb sram
//goto pdf reference: http://www.nuvoton.com/resource-files/TRM_NUC123_Series_EN_Rev2.04.pdf

//Note USB supports up to 8 endpoints

void SystemInit(){
    //section: Clock configurationta

    //Unlock configuration registers
    SYS_UnlockReg();
    //Configure clock for periphials, pll, ...
    CLK_T* ClockContrMemoryMap=(CLK_T*) CLK_BASE_POINTER;
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
    //?TODO use divider to half HCLK

    //Relock configuration registers
    SYS_LockReg();



    //PIN CONFIGURATION

    //Special Pin config
    //I2C1 - SCL PA11, SDA PA10
    //UART0 - RXD PC4, TXD PC5
    GPA_MFP=0x00000C00; //I2C1 SCL and SDA
    GPB_MFP=0x00000002; //UART0 Rx and Tx
    ALT_MFP=0x60000000;

    //GPIO Config (input 0b00 (in), push pull (out) 0b01, opendrain 0b10, bidirect 0b11 (default))
    //TODO bluetooth 3 version PC2 and PC3 configuration as input/output?
    GPIOA_PMD=0x41FFFFFF; //pa12 out backlEN, pa13 in matrix, pa14 in battery chrg, pa15 out matrix
    GPIOB_PMD=0xCFD4FFFF; //pb8 in matrix, pb9 out matrix, pb10 out shift_clk, pb14 in powerSRC?
    GPIOC_PMD=0xF555FFF5; //pc0 out for shift_rclk, pc1 out shift_data, pc8-pc13 out matrix
    GPIOD_PMD=0xFFFFF000; //pd0-pd5 in matrix
    GPIOF_PMD=0x0000000F; //pf2 in matrix,pf3 in powerSRC?

    //TODO reset usb (USBD_RST IPRSTC2[27])

}
int main(void){
    keydata keys[8][9]={0};
    uint8_t val=0;
    while(1){
        val++;
        setRGB_row(keys);
        for(int row=0;row<9;row++){
            for(int col=0;col<8;col++){
                keys[col][row].green=val;
            }
        }
    }

}
