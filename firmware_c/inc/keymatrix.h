#ifndef KEYMATRIX_H_INCLUDED
#define KEYMATRIX_H_INCLUDED
#include "custom.h"

const uint32_t* rowOUTPointer[9]={
    GPIOD0_DOUT_POINTER,
    GPIOD1_DOUT_POINTER,
    GPIOD2_DOUT_POINTER,
    GPIOD3_DOUT_POINTER,
    GPIOD4_DOUT_POINTER,
    GPIOD5_DOUT_POINTER,
    GPIOF2_DOUT_POINTER,
    GPIOA13_DOUT_POINTER,
    GPIOB8_DOUT_POINTER
};

const uint32_t* colINPointer[8]={
    GPIOB9_DOUT_POINTER,
    GPIOC13_DOUT_POINTER,
    GPIOC12_DOUT_POINTER,
    GPIOC11_DOUT_POINTER,
    GPIOC10_DOUT_POINTER,
    GPIOC9_DOUT_POINTER,
    GPIOC8_DOUT_POINTER,
    GPIOA15_DOUT_POINTER
};

void getPressedKeys(keydata KeyDataListIN[8][9]){
    for(uint8_t currentrow=0;currentrow<9;currentrow++){
        uint32_t* outputAddressPointer;
        outputAddressPointer=(uint32_t*)rowOUTPointer[currentrow];
        *outputAddressPointer=0; //pull row low
        for(uint8_t currentcol=0;currentcol<8;currentcol++){//Read Columns
            uint32_t* inputAddressPointer;
            inputAddressPointer=(uint32_t*)colINPointer[currentcol];
            if(!(*inputAddressPointer)){ //key is pressed
                KeyDataListIN[currentcol][currentrow].green=255;
            }else{ //key is released
                KeyDataListIN[currentcol][currentrow].green=0;
            }
        }
        *outputAddressPointer=1;
    }
}

#endif // KEYMATRIX_H_INCLUDED
