#ifndef BACKLIGHT_H_INCLUDED
#define BACKLIGHT_H_INCLUDED

#include "custom.h"

typedef struct{
    uint8_t state;
    uint8_t red;
    uint8_t red_temp_brightness;
    uint8_t green;
    uint8_t green_temp_brightness;
    uint8_t blue;
    uint8_t blue_temp_brightness;
}keydata;


void backlight_power_on();
void backlight_power_off();
void shift_one_bit();

void delay(uint32_t delaycyc){
    for(volatile uint32_t step=0;step<delaycyc;step++){
    }
}

void enable_column(uint8_t colnum_to_enable){
    GPIOC1_DOUT=1;//bring output into defined state
    for(uint8_t currentcolnum=0;currentcolnum<8;currentcolnum++){
        if(colnum_to_enable==currentcolnum){
            //LED shift on bit one place
            GPIOC1_DOUT=0;
            shift_one_bit();
            //LED off bit to prepare for the else case which gets called more frequently
            GPIOC1_DOUT=1;
        }else{
            shift_one_bit();
        }
    }
}

void setRGB(keydata KeyDataListIN[8][9]){
    static uint8_t currentcolnum=0;
    backlight_power_off();
    for(uint8_t currentrow=0;currentrow<9;currentrow++){ //Loop for green
        KeyDataListIN[currentcolnum][currentrow].green_temp_brightness+=KeyDataListIN[currentcolnum][currentrow].green;
        //to equaly spread the on/off cycles this variable is used to store a overflowing uint8
        if(KeyDataListIN[currentcolnum][currentrow].green_temp_brightness<KeyDataListIN[currentcolnum][currentrow].green){ //green_temp_brightness has wrapped around enable led
            GPIOC1_DOUT=0; //Set data line on
            shift_one_bit();
        }else{
            GPIOC1_DOUT=1; //Set data line off
            shift_one_bit();
        }
    }
    for(uint8_t currentrow=0;currentrow<9;currentrow++){ //Loop over blue
        KeyDataListIN[currentcolnum][currentrow].blue_temp_brightness+=KeyDataListIN[currentcolnum][currentrow].blue;
        if(KeyDataListIN[currentcolnum][currentrow].blue_temp_brightness<KeyDataListIN[currentcolnum][currentrow].blue){
            GPIOC1_DOUT=0; //Set data line on
            shift_one_bit();
        }else{
            GPIOC1_DOUT=1; //Set data line off
            shift_one_bit();
        }
    }
    for(uint8_t currentrow=0;currentrow<9;currentrow++){ //Loop over red
        KeyDataListIN[currentcolnum][currentrow].red_temp_brightness+=KeyDataListIN[currentcolnum][currentrow].red;
        if(KeyDataListIN[currentcolnum][currentrow].red_temp_brightness<KeyDataListIN[currentcolnum][currentrow].red){
            GPIOC1_DOUT=0; //Set data line on
            shift_one_bit();
        }else{
            GPIOC1_DOUT=1; //Set data line off
            shift_one_bit();
        }
    }
    enable_column(currentcolnum); //TODO time this with a timer
    backlight_power_on();
    currentcolnum++;
    if(currentcolnum>8){
        currentcolnum=0;
    }
}

__inline void shift_one_bit(){
    GPIOB10_DOUT=1; //shift through registers with clk
    GPIOB10_DOUT=0;

    GPIOC0_DOUT=1; //Pule rclk to display result on shift register output
    GPIOC0_DOUT=0;
}

__inline void backlight_power_off(){
    GPIOA12_DOUT=0; //set PA12 low
}

__inline void backlight_power_on(){
    GPIOA12_DOUT=1; //set PA12 high
}



#endif // BACKLIGHT_H_INCLUDED

