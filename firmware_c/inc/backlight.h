#ifndef BACKLIGHT_H_INCLUDED
#define BACKLIGHT_H_INCLUDED

#include "custom.h"

typedef struct{
    uint8_t state;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}keydata;

/*Column Layout
┌────────────────────────────────────────────────────┐
│ 0  1  2  3  4  5  6  7  0  1  2  3  4   5  6  7  4 │
│ 0   1  2  3  4  5  6  7  0  1  2  3  4  5  6  7  5 │
│  0   1  2  3  4  5  6  7  0  1  2  3   4           │
│ 0  ?  1  2  3  4  5  6  3  0  1  2    3       6    │
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

/*keydata keyMatrix[8][9]={ //Column, then Row
    {0}
};*/

void enable_column(uint8_t colnum_to_enable){
    GPIOC1_DOUT=0;//bring output into defined state
    for(uint8_t currentcolnum=0;currentcolnum<8;currentcolnum++){
        if(colnum_to_enable==currentcolnum){
            //LED shift on bit one place
            GPIOC1_DOUT=1;
            //shift register clock pulse
            GPIOB10_DOUT=0;
            GPIOB10_DOUT=1;
            /*enable rclk, absolutely necessary because there might still be data inside
            the shift register for the colum which needs to get passed to the ones for color*/
            GPIOC0_DOUT=0;
            GPIOC0_DOUT=1;
            //LED off bit to prepare for the else case which gets called more frequently
            GPIOC1_DOUT=0;
        }else{
            //No need to set data as it stays low and is set low in if case.
            //Shift off bit one place
            GPIOB10_DOUT=0;
            GPIOB10_DOUT=1;
            /*enable rclk, absolutely necessary because there might still be data inside
            the shift register for the colum which needs to get passed to the ones for color*/
            GPIOC0_DOUT=0;
            GPIOC0_DOUT=1;
        }
    }
}

void setRGB_row(keydata KeyDataListIN[8][9]){
    static uint8_t rgb_timer=0;
    uint8_t rgb_time_mask=(1<<rgb_timer++);
    if(rgb_timer>7){
        rgb_timer=0;
    }
    for(uint8_t currentcolnum=0;currentcolnum<8;currentcolnum++){ //and
        for(uint8_t currentrow=0;currentrow<9;currentrow++){ //Loop for red
            if(KeyDataListIN[currentrow][currentcolnum].red&rgb_time_mask){
                GPIOC1_DOUT=1; //Set data line on

                GPIOB10_DOUT=0; //shift through registers with clk
                GPIOB10_DOUT=1;

                GPIOC0_DOUT=0; //Pule rclk to display result on shift register output
                GPIOC0_DOUT=1;
            }else{
                GPIOC1_DOUT=0; //Set data line off

                GPIOB10_DOUT=0; //shift through registers with clk
                GPIOB10_DOUT=1;

                GPIOC0_DOUT=0; //Pule rclk to display result on shift register output
                GPIOC0_DOUT=1;
            }
        }
        for(uint8_t currentrow=0;currentrow<9;currentrow++){ //Loop over green
            if(KeyDataListIN[currentrow][currentcolnum].green&rgb_time_mask){
                GPIOC1_DOUT=1; //Set data line on

                GPIOB10_DOUT=0; //shift through registers with clk
                GPIOB10_DOUT=1;

                GPIOC0_DOUT=0; //Pule rclk to display result on shift register output
                GPIOC0_DOUT=1;
            }else{
                GPIOC1_DOUT=0; //Set data line off

                GPIOB10_DOUT=0; //shift through registers with clk
                GPIOB10_DOUT=1;

                GPIOC0_DOUT=0; //Pule rclk to display result on shift register output
                GPIOC0_DOUT=1;
            }
        }
        for(uint8_t currentrow=0;currentrow<9;currentrow++){ //Loop over blue
            if(KeyDataListIN[currentrow][currentcolnum].blue&rgb_time_mask){
                GPIOC1_DOUT=1; //Set data line on

                GPIOB10_DOUT=0; //shift through registers with clk
                GPIOB10_DOUT=1;

                GPIOC0_DOUT=0; //Pule rclk to display result on shift register output
                GPIOC0_DOUT=1;
            }else{
                GPIOC1_DOUT=0; //Set data line off

                GPIOB10_DOUT=0; //shift through registers with clk
                GPIOB10_DOUT=1;

                GPIOC0_DOUT=0; //Pule rclk to display result on shift register output
                GPIOC0_DOUT=1;
            }
        }
        enable_column(currentcolnum); //TODO time this with a timer
    }
}

__inline void backlight_power_off(){
    GPIOA12_DOUT=0; //set PA12 low
}

__inline void backlight_power_on(){
    GPIOA12_DOUT=1; //set PA12 high
}

#endif // BACKLIGHT_H_INCLUDED

