#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#define TMR_BA01 0x40010000
#define TMR_BA23 0x40110000

#define TCSR0  (*((uint32_t*)(TMR_BA01+0x00))) //Control and Status register
#define TCMPR0 (*((uint32_t*)(TMR_BA01+0x04))) //Compare Register (lower 3 bytes)
#define TDR0   (*((uint32_t*)(TMR_BA01+0x0C))) //counter value



/*Timers are used for the keyboard matrix readout callback and the backlight usinge the TIF interrupt
TMRß os ised for generating interrupts for led's and keymatrix
*/

__inline void timer0_init(uint8_t prescaler){
    TCSR0=0x28000000+prescaler; //interrupt on, periodic,
}
#define timer0_start() TCSR0|=0x40000000
#define timer0_stop()  TCSR0&=0xBFFFFFFF
__inline void timer0_set_compare(uint32_t compare_val){
    TCMPR0=compare_val&0x00FFFFFF;
}

#endif // TIMER_H_INCLUDED
