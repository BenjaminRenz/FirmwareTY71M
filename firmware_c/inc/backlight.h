#ifndef BACKLIGHT_H_INCLUDED
#define BACKLIGHT_H_INCLUDED
//memory base address of the gpio controller
#define GP_BA 0x50004000

#define GPIOA12_DOUT (GP_BA+0x230)
#define GPIOB10_DOUT (GP_BA+0x268)
#define GPIOC0_DOUT  (GP_BA+0x280)
#define GPIOC1_DOUT  (GP_BA+0x284)

#define backlight_enable_output ((volatile unsigned int *) GPIOA12_DOUT   //high backlight on, low backlight off
#define backlight_shift_clk     ((volatile unsigned int *) GPIOB10_DOUT   //clock to shift through the rgister
#define backlight_shift_rclk    ((volatile unsigned int *) GPIOC0_DOUT    //pull low then high to display internal shifted values on the output pins
#define backlight_shift_data    ((volatile unsigned int *) GPIOC1_DOUT    //connected to data in pin on first shift register



void enable_column(uint8_t colnum_to_enable){
    backlight_shift_data=0;//bring output into defined state
    for(uint8_t currentcolnum=0,currentcolnum<8,currentcolnum++){
        if(colnum_to_enable==currentcolnum){
            //LED shift on bit one place
            backlight_shift_data=1;
            //shift register clock pulse
            backlight_shift_clk=0;
            backlight_shift_clk=1;
            /*enable rclk, absolutely necessary because there might still be data inside
            the shift register for the colum which needs to get passed to the ones for color*/
            backlight_shift_rclk=0;
            backlight_shift_rclk=1;
            //LED off bit to prepare for the else case which gets called more frequently
            backlight_shift_data=0;
        }else{
            //No need to set data as it stays low and is set low in if case.
            //Shift off bit one place
            backlight_shift_clk=0;
            backlight_shift_clk=1;
            /*enable rclk, absolutely necessary because there might still be data inside
            the shift register for the colum which needs to get passed to the ones for color*/
            backlight_shift_rclk=0;
            backlight_shift_rclk=1;
        }
    }
}

void setRGB_row(){

}

__inline void backlight_power_off(){
    backlight_enable_output=0; //set PA12 low
}

__inline void backlight_power_on(){
    backlight_enable_output=1; //set PA12 high
}

#endif // BACKLIGHT_H_INCLUDED

