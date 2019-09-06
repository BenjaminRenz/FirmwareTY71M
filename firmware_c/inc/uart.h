//Guess for the bluetooth module is 9600 baud

//Flow controll disabeled
//8+1
//PLLFOUT is 144MHz to get 9600baud we need a divider of:
//(144 000 000)/(9 600)=(15 000)
//we chose Mode 1 over Mode 0 because (15000%16!=0)
//DIVIDER_X =14, BRD=998
//DIVIDER_X=0xE
//BRD=0x3E6
//-> Baudrate (144 000 000)/((DIVIDER_X+1)*(BRD+2))=9600
#include "custom.h"

#define UART_BA(UartN) (0x40050000+0x10000*UartN)
#define UART0_IER (*((uint32_t*)(UART_BA(0)+0x04)))
#define UART0_LCR (*((uint32_t*)(UART_BA(0)+0x0c)))
#define UART0_RBR (*((uint32_t*)(UART_BA(0))))
#define UART0_THR (*((uint32_t*)(UART_BA(0))))
#define UART0_MCR (*((uint32_t*)(UART_BA(0)+0x10)))
#define UART0_FSR (*((uint32_t*)(UART_BA(0)+0x18)))
#define UART0_ISR (*((uint32_t*)(UART_BA(0)+0x1c)))
#define UART0_BAUD (*((uint32_t*)(UART_BA(0)+0x24)))
#define UART0_FCR (*((uint32_t*)(UART_BA(0)+0x08)))  //high bytes overlap with MCR register

uint8_t* UART0_rx_pointer=0;
uint32_t UART0_rx_recieved_bytes=0;
uint32_t UART0_tx_bytes_left=0;
uint32_t UART0_tx_total_to_send=0;
uint8_t* UART0_tx_pointer=0;

void UART0_IRQHandler(){
    if(UART0_ISR&(1<<8)){ //rx data over treshold
        while(!(UART0_FSR&(1<<14))){ //rx not empty
            UART0_rx_pointer[UART0_rx_recieved_bytes++]=UART0_RBR;
        }
    }else if(UART0_ISR&(1<<9)){ //tx empty
        while(UART0_tx_bytes_left>0&&!(UART0_FSR&(1<<23))){ //still data to send and buffer not full
            UART0_THR=UART0_tx_pointer[UART0_tx_total_to_send-(UART0_tx_bytes_left--)];
        }
        if(!UART0_tx_bytes_left){
            UART0_IER&=0xfffffffd; //disable tx empty interrupt, otherwise we will end up hanging inside this interrupt
        }
    }else{
        debugr=255; //should not happen
    }
}

void UART0_init(){
    UART0_start_reset();
    UART0_end_reset();
    UART0_BAUD=0x2e0003e6; //div_x_en, div_x=14, brd=998 -> 9600 baud
    UART0_LCR =0x00000003; //8n1
    UART0_FCR&=0xfffffe0f; //set rxtreshold to 1byte, enable rx
    UART0_IER|=0x00000001; //enable interrupts for tx empty, rxdata over treshold
}

void UART0_send_async(uint8_t* data,uint32_t bytesToSend,uint32_t* bytesLeft){
    UART0_tx_total_to_send=bytesToSend;
    UART0_tx_bytes_left   =bytesToSend;
    if(bytesLeft){
        bytesLeft=&UART0_tx_bytes_left; //inform the user how many bytes still need to be transmitted
    }
    UART0_tx_pointer=data;
    while(UART0_tx_bytes_left>0&&!(UART0_FSR&(1<<23))){ //first buffer fill
        UART0_THR=UART0_tx_pointer[UART0_tx_total_to_send-(UART0_tx_bytes_left--)];
    }
    UART0_IER|=0x00000002; //enable tx empty interrupt
    //now let the interrupt routine handle the transfer
}

void UART0_send_and_wait(uint8_t* data, uint32_t bytesToSend){
    uint32_t UART0_tx_bytes_left=bytesToSend;
    while(UART0_tx_bytes_left>0){
        if((UART0_FSR&(1<<23))){ //FIFO for rx full, wait

        }else{
            UART0_THR=data[bytesToSend-(UART0_tx_bytes_left--)];
        }
    }
}

void print32HEX(uint32_t ToHex){
    uint8_t hexstring[12];
    hexstring[0]='0';
    hexstring[1]='x';
    for(uint8_t i=0;i<8;i++){
        uint8_t isolatedBlock=((ToHex&(0xf0000000>>(4*i)))>>((28-4*i)));
        if(isolatedBlock<10){
            hexstring[2+i]=0x30+isolatedBlock;
        }else{
            hexstring[2+i]=0x57+isolatedBlock;
        }
    }
    hexstring[10]=' ';
    hexstring[10]='\n';
    hexstring[12]=0;
    UART0_send_async(hexstring,13,0);//TODO potentialy buggy, if called to frequently
}

void print8shortHEX(uint8_t ToHex){
    uint8_t hexstring[3];
    hexstring[0]='x';
    for(uint8_t i=0;i<2;i++){
        uint8_t isolatedBlock=((ToHex&(0xf0>>(4*i)))>>((4-4*i)));
        if(isolatedBlock<10){
            hexstring[1+i]=0x30+isolatedBlock;
        }else{
            hexstring[1+i]=0x57+isolatedBlock;
        }
    }
    UART0_send_async(hexstring,3,0);//TODO potentialy buggy, if called to frequently
}
