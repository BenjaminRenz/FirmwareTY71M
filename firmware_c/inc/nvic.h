#ifndef NVIC_H_INCLUDED
#define NVIC_H_INCLUDED

#define SCS_BA 0xE000E000
#define NVIC_IPR3 (*((uint32_t*)(USB_BA+0x40C))) //interrupt priority for IRQ12-15 (for UART0)
#define NVIC_IPR4 (*((uint32_t*)(USB_BA+0x410))) //interrupt priority for IRQ16-19 (for I2C1)
#define NVIC_IPR5 (*((uint32_t*)(USB_BA+0x414))) //interrupt priority for IRQ20-23 (for USB)
#define NVIC_IPR6 (*((uint32_t*)(USB_BA+0x418))) //interrupt priority for IRQ24-27 (for DMA)
/*
    IRQ12 is UART0 interrupt
    IRQ19 is I2C1 interrupt
    IRQ23 is USB interrupt
    IRQ26 is DMA interrupt
*/

__inline void NVIC_init(){

}


void I2C1_IRQHandler(void);
#endif // NVIC_H_INCLUDED
