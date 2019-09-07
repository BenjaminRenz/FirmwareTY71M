#ifndef KEYMATRIX_H_INCLUDED
#define KEYMATRIX_H_INCLUDED
#include "custom.h"
#include "keyCodes.h"
/*Column Layout
┌────────────────────────────────────────────────────┐
│ 0  1  2  3  4  5  6  7  0  1  2  3  4   5  6  7  4 │
│ 0   1  2  3  4  5  6  7  0  1  2  3  4  5  6  7  5 │
│  0   1  2  3  4  5  6  7  0  1  2  3   4           │
│ 0  7  1  2  3  4  5  6  7  0  1  2    3       6    │
│ 0  1  2  ---------3--------  4  5  6   7   5  6  7 │
└────────────────────────────────────────────────────┘
*/

/*Row Layout
┌────────────────────────────────────────────────────┐
│ 0  0  0  0  0  0  0  0  5  5  5  5  5   5  5  5  8 │
│ 1   1  1  1  1  1  1  1  6  6  6  6  6  6  6  6  8 │
│  2   2  2  2  2  2  2  2  7  7  7  7   7           │
│ 3  8  3  3  3  3  3  3  3  8  8  8    8       8    │
│ 4  4  4  ---------4--------  4  4  4   4   7  7  7 │
└────────────────────────────────────────────────────┘
*/



const uint32_t* rowOUTPointer[9]={  //array of pointers, filled with memory addresses for the corresponding direct gpio bitband register
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

const uint32_t* colINPointer[8]={  //array of pointers, filled with memory addresses for the corresponding direct gpio bitband register
    GPIOB9_DOUT_POINTER,
    GPIOC13_DOUT_POINTER,
    GPIOC12_DOUT_POINTER,
    GPIOC11_DOUT_POINTER,
    GPIOC10_DOUT_POINTER,
    GPIOC9_DOUT_POINTER,
    GPIOC8_DOUT_POINTER,
    GPIOA15_DOUT_POINTER
};

const uint8_t keymatrix_bootmode_normal[8][9]={//outer index corresponds to column (therefore move one row up or down below), the inner index is the row (therefore move right left in the list below) according to layout see above
    {KEY_ESCAPE ,KEY_TAB    ,KEY_CAPSLOCK   ,KEY_MOD_LEFTSHIFT  ,KEY_MOD_LEFTCTRL   ,KEY_8      ,KEY_I              ,KEY_K          ,KEY_COMMA          },
    {KEY_1      ,KEY_Q      ,KEY_A          ,KEY_Z              ,KEY_MOD_LEFTOS     ,KEY_9      ,KEY_O              ,KEY_L          ,KEY_POINT          },
    {KEY_2      ,KEY_W      ,KEY_S          ,KEY_X              ,KEY_MOD_LEFTALT    ,KEY_0      ,KEY_P              ,KEY_SEMIKOLON  ,KEY_SLASH          },
    {KEY_3      ,KEY_E      ,KEY_D          ,KEY_C              ,KEY_SPACEBAR       ,KEY_MINUS  ,KEY_LEFTBRACE      ,KEY_APOSTROPHE ,KEY_MOD_RIGHTSHIFT },
    {KEY_4      ,KEY_R      ,KEY_F          ,KEY_V              ,KEY_MOD_RIGHTALT   ,KEY_EQUAL  ,KEY_RIGHTBRACE     ,KEY_RETURN     ,KEY_PAGEUP         },
    {KEY_5      ,KEY_T      ,KEY_G          ,KEY_B              ,KEY_MOD_RIGHTOS    ,KEY_DELETE ,KEY_BACKSLASH      ,KEY_LEFTARROW  ,KEY_PAGEDOWN       },
    {KEY_6      ,KEY_Y      ,KEY_H          ,KEY_N              ,KEY_APPLICATION    ,KEY_INSERT ,KEY_DELETEFORWARD  ,KEY_DOWNARROW  ,KEY_UPARROW        },
    {KEY_7      ,KEY_U      ,KEY_J          ,KEY_M              ,KEY_MOD_RIGHTCTRL  ,KEY_HOME   ,KEY_END            ,KEY_RIGHTARROW ,KEY_BACKSLASHNONUS }
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
                KeyDataListIN[currentcol][currentrow].state=1;
                KeyDataListIN[currentcol][currentrow].green=255; //TODO add function here to further process keypress
            }else{ //key is released
                KeyDataListIN[currentcol][currentrow].green=0;
                KeyDataListIN[currentcol][currentrow].state=0;
            }
        }
        *outputAddressPointer=1;
    }
}
enum {boot_protocol,report_protocol};
void reportPressedKeys(uint32_t protocol,keydata KeyDataListIn[8][9],uint8_t* report_out){
    if(protocol==boot_protocol){
        uint32_t keycodebyte=2; //Start at the third byte with the keycodes because [0] is modifier and [1] is reserved
        uint32_t currentrow=0;
        uint32_t currentcol=0;
        for(uint8_t i=0;i<8;i++){
            report_out[i]=0;
        }
        while((currentrow<9)&&(keycodebyte<8)){     //break if we have finished or if we have more than 6 key rollover
            uint8_t keycode=KEY_NONE;
            if(KeyDataListIn[currentcol][currentrow].state){ //if key is pressed
                keycode=keymatrix_bootmode_normal[currentcol][currentrow];
            }
            if(keycode==KEY_MOD_LEFTCTRL){
                report_out[0] &=0x01;
            }else if(keycode==KEY_MOD_LEFTSHIFT){
                report_out[0] &=0x02;
            }else if(keycode==KEY_MOD_LEFTALT){
                report_out[0] &=0x04;
            }else if(keycode==KEY_MOD_LEFTOS){
                report_out[0] &=0x08;
            }else if(keycode==KEY_MOD_RIGHTCTRL){
                report_out[0] &=0x10;
            }else if(keycode==KEY_MOD_RIGHTSHIFT){
                report_out[0] &=0x20;
            }else if(keycode==KEY_MOD_RIGHTALT){
                report_out[0] &=0x40;
            }else if(keycode==KEY_MOD_RIGHTOS){
                report_out[0] &=0x80;
            }else if(keycode!=KEY_NONE){//Normal key registered
                //TODO don't asume that a key can not be defined twice, so check previous codes and check for duplicates
                report_out[keycodebyte++]=keycode;
            }
            currentcol++;   //Is equivalent to two nested for loop iterating over the whole key matrix
            if(currentcol>7){
                currentcol=0;
                currentrow++;
            }

        }
    }
}

#endif // KEYMATRIX_H_INCLUDED
