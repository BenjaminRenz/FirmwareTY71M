#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED
uint8_t USB_initiate_send(uint32_t epnum, uint8_t* data, uint32_t packetLength, void (*callbFuncP)(uint32_t epnum,uint8_t* data, uint32_t packetlength));
void UART0_send_async(char* data,uint32_t bytesToSend,uint32_t* bytesLeft);

#define TMR_BA01 0x40010000
#define TMR_BA23 0x40110000

#define TCSR0  (*((uint32_t*)(TMR_BA01+0x00))) //Control and Status register
#define TCMPR0 (*((uint32_t*)(TMR_BA01+0x04))) //Compare Register (lower 3 bytes)
#define TDR0   (*((uint32_t*)(TMR_BA01+0x0C))) //counter value
#define TISR0  (*((uint32_t*)(TMR_BA01+0x08))) //Timer interrupt status register, needed to clear interrupt
#define TCSR1  (*((uint32_t*)(TMR_BA01+0x20))) //Control and Status register
#define TCMPR1 (*((uint32_t*)(TMR_BA01+0x24))) //Compare Register (lower 3 bytes)
#define TDR1   (*((uint32_t*)(TMR_BA01+0x2C))) //counter value
#define TISR1  (*((uint32_t*)(TMR_BA01+0x28))) //Timer interrupt status register, needed to clear interrupt
#define TCSR2  (*((uint32_t*)(TMR_BA23+0x00))) //Control and Status register
#define TCMPR2 (*((uint32_t*)(TMR_BA23+0x04))) //Compare Register (lower 3 bytes)
#define TDR2   (*((uint32_t*)(TMR_BA23+0x0C))) //counter value
#define TISR2  (*((uint32_t*)(TMR_BA23+0x08))) //Timer interrupt status register, needed to clear interrupt
#define TCSR3  (*((uint32_t*)(TMR_BA23+0x20))) //Control and Status register
#define TCMPR3 (*((uint32_t*)(TMR_BA23+0x24))) //Compare Register (lower 3 bytes)
#define TDR3   (*((uint32_t*)(TMR_BA23+0x2C))) //counter value
#define TISR3  (*((uint32_t*)(TMR_BA23+0x28))) //Timer interrupt status register, needed to clear interrupt

/*Timer usage
TIMER 0 -> key pressed readout and rgb refresh at constant 1000Hz
TIMER 1 -> USB hid set idle and poll rate
*/

void timer0_init(uint8_t prescaler){ //prescaler is multiplying input 1/(prescaler+1)
    TCSR0=0x28000000+prescaler; //interrupt on, periodic,
}
void timer1_init(uint8_t prescaler){ //prescaler is multiplying input 1/(prescaler+1)
    TCSR1=0x28000000+prescaler; //interrupt on, periodic,
}
void timer2_init(uint8_t prescaler){ //prescaler is multiplying input 1/(prescaler+1)
    TCSR2=0x28000000+prescaler; //interrupt on, periodic,
}
void timer3_init(uint8_t prescaler){ //prescaler is multiplying input 1/(prescaler+1)
    TCSR3=0x28000000+prescaler; //interrupt on, periodic,
}
#define timer0_start() TCSR0|=0x40000000
#define timer1_start() TCSR1|=0x40000000
#define timer2_start() TCSR2|=0x40000000
#define timer3_start() TCSR3|=0x40000000
#define timer0_stop()  TCSR0&=0xBFFFFFFF
void timer0_set_compare(uint32_t compare_val){
    TCMPR0=compare_val&0x00FFFFFF;
}
void timer1_set_compare(uint32_t compare_val){
    TCMPR1=compare_val&0x00FFFFFF;
}
void timer2_set_compare(uint32_t compare_val){
    TCMPR2=compare_val&0x00FFFFFF;
}
void timer3_set_compare(uint32_t compare_val){
    TCMPR3=compare_val&0x00FFFFFF;
}

void TMR0_IRQHandler(){     //keydata_and_rgb
    static uint8_t increment=0;
    TISR0=0x00000001;  //Clear interrupt1
    getPressedKeys(keys);
    if(!increment){
        keys[0][0].red++;
        keys[1][0].blue++;
    }
    increment++;
    setRGB(keys);
}

void TMR1_IRQHandler(){     //new data must be pushed for usb hid report
    TISR1=0x00000001;  //Clear interrupt1
    for(uint8_t i=0;i<8;i++){   //backup old message
        previous_report_out[i]=report_hid_out[i];
    }
    reportPressedKeys(0,keys,report_hid_out);
    uint8_t difference_flag=0;
    for(uint8_t i=0;i<8;i++){
        if(previous_report_out[i]!=report_hid_out[i]){
            difference_flag=1;
        }
    }
    if(difference_flag){    //TODO check if set idle is infinite (device should only reply if keys changed, or if periodic polling is active)
        if(USB_initiate_send(2,report_hid_out,8,NULL)){
            UART0_send_async("+",1,0);  //commited to buffer
        }else{
            UART0_send_async("-",1,0); //busy, not sent
        }
    }

}
void TMR2_IRQHandler(){
    TISR2=0x00000001;  //Clear interrupt1
}
void TMR3_IRQHandler(){
    TISR3=0x00000001;  //Clear interrupt1
}

#endif // TIMER_H_INCLUDED


