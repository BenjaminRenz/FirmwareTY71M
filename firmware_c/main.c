#define __NO_SYSTEM_INIT
#include "custom.h"    //NUC123LD4AN hardware register definitions
#include <stdlib.h>
#include "backlight.h" //functions for setting the rgb led backlight

#include "keymatrix.h" //functions for scanning the key matrix
keydata keys[8][9]={0};
volatile uint8_t debugr=0;
volatile uint8_t debugg=0;
volatile uint8_t debugb=0;
#include "uart.h"

#include "usb.h"       //usb support
//Note USB supports up to 8 endpoints
#include "nvic.h"      //Interrupt controller needed for usb


#include "timer.h"

//microcontroller is a NUC123LD4AN with 68kb flash and 20kb sram
//goto pdf reference:
/*
http://www.nuvoton.com/resource-files/TRM_NUC123_Series_EN_Rev2.04.pdf
*/

void SystemInit(){  //DANGER, DON'T CREATE VARIABLES HERE, SEE WARNING BELOW !!!!
    //TODO use CONFIG0 for all of this in the future
    //Enable XTAL pins to use external clock
    GPF_MFP=0x00000003; //Xtal pins, transformed to IO-pins after reboot, so enable them before clock switching
    //SECTION CLOCK CONFIGURATION
    //Unlock configuration registers
    SYS_UnlockReg();
    //Configure clock for periphials, pll, ...
    CLK_T* ClockContrMemoryMap=(CLK_T*) CLK_BASE_POINTER;
    ClockContrMemoryMap->AHBCLK=CLK_AHBCLK_ISP_Msk;                                             //TODO check if (ISP clock) required
    ClockContrMemoryMap->APBCLK=CLK_APBCLK_USBD_Msk|CLK_APBCLK_I2C1_Msk|CLK_APBCLK_UART0_Msk;   //Clock for usb,i2c1 and uart0 enabled
    ClockContrMemoryMap->CLKDIV=CLK_CLKDIV_USB(3)|CLK_CLKDIV_HCLK(2);
    ClockContrMemoryMap->PLLCON=70;                                                             //FB_DV=70, OUT_DV=0, IN_DV=0, PLL_SRC=HXT
    ClockContrMemoryMap->CLKSEL1=CLK_CLKSEL1_WDT_S_HCLK_DIV2048|CLK_CLKSEL1_UART_S_PLL;
    //ClockContrMemoryMap->CLKSEL2=//CLK_CLKSEL2
    //Enable external HXT
    ClockContrMemoryMap->PWRCON=CLK_PWRCON_XTL12M_EN_Msk|CLK_PWRCON_OSC22M_EN_Msk|CLK_PWRCON_PD_WU_DLY_Msk|CLK_PWRCON_PD_WU_INT_EN_Msk;
    //Wait for stability of external and internal oscillator
    while(((~ClockContrMemoryMap->CLKSTATUS)&(CLK_CLKSTATUS_OSC22M_STB_Msk|CLK_CLKSTATUS_XTL12M_STB_Msk|CLK_CLKSTATUS_PLL_STB_Msk))){ //while any of the clocks is unstable
    }
    //Set clock to external
    ClockContrMemoryMap->CLKSEL0=CLK_CLKSEL0_STCLK_S_HCLK_DIV2|CLK_CLKSEL0_HCLK_S_PLL; //Because of conflicting documentation of CLK_CLKSEL0_HCLK_S_LXT I will use CLK_CLKSEL_HCLK_S_PLL, reference says this is PLL/2 but programming library says otherwise
    //Relock configuration registers
    SYS_LockReg();


    //SECTION PIN CONFIGURATION

    //Special Pin config
    //I2C1 - SCL PA11, SDA PA10
    //UART0 - RXD PC4, TXD PC5
    GPA_MFP=0x00000C00; //I2C1 SCL and SDA
    GPB_MFP=0x00000000;
    GPC_MFP=0x00000020; //UART0 Rx and Tx
    GPD_MFP=0x00000000;
    GPF_MFP=0x00000003; //Enable Xtal pins for external 4MHz Quartz
    ALT_MFP=0x60000000;

    //GPIO Config (input 0b00 (in), push pull (out) 0b01, opendrain 0b10, bidirect 0b11 (default))
    //TODO bluetooth 3.x version PC2 and PC3 configuration as input/output?
    GPIOA_PMD=0x05FFFFFF; //pa12 out backlEN, pa13 out matrix, pa14 in battery chrg, pa15 in matrix
    GPIOB_PMD=0xCFD1FFFF; //pb8 out matrix, pb9 in matrix, pb10 out shift_clk, pb14 in powerSRC?
    GPIOC_PMD=0xF000FFF5; //pc0 out for shift_rclk, pc1 out shift_data, pc8-pc13 in matrix
    GPIOD_PMD=0xFFFFF555; //pd0-pd5 out matrix
    GPIOF_PMD=0x0000001F; //pf2 out matrix,pf3 in powerSRC?
    //TODO configure clock for i2c

    //WARNING !!!! DO NOT INITIALIZE ANY VARIABLES HERE, THEY WILL BE DELETED, EVEN GLOBALS, AND DO NOT CALL FUNCTIONS THAT TRY TO DO SO !!!!
    // the .data sections are still going to be initialized, so any global variable will get deleted, see here:
    //https://github.com/ARM-software/CMSIS_5/issues/405
}

int main(void){
    //Configure peripherals
    USB_init();
    NVIC_init(); //Should be after USB because USB needs to initialize first
    //USART0_start_reset();
    UART0_init();
    //I2C1_init();
    USB_clear_se0(); //Start USB communication by clearing bus reset
    while(1){
        if(debugb){
            debugb--;
        }
        if(debugr){
            debugr--;
        }
        if(debugg){
            debugg--;
        }
        /*for(int row=0;row<9;row++){
            for(int col=0;col<8;col++){
                keys[col][row].blue=debugb;
                keys[col][row].red=debugr;
                keys[col][row].green=debugg;
            }
        }*/
        setRGB(keys);
        getPressedKeys(keys);
    }
    //For send keys we need to send modifier as keys if no other key is pressed, so our pc can register them without any other keys
}
