#Clock configuration thoughts
External Quartz (HXT) is 4MHz
Internal High speed (HIRC) is 22.1MHz +/-3%
USB needs 48MHz
Max cortex m0 core speed is 72MHz
PLL max is 144MHz


FIN=4MHz
Set IN_DV=0       -> NR=2  -> can't change with 4MHz HXT because of Fref restriction
Set OUT_DV=0      -> NO=1
Set FB_DV=70      -> NF=72

Constraints:
-4MHz<FIN<24MHz                             -> checked
-0.8MHz<Fref<8MHz                           -> checked
-100MHz<FCO<200MHz better 120MHz<FCO<200MHz -> checked

Fref=FIN/2/NR=4MHz/2/2=1MHz
FCO=Fref*2*NF=1MHz*2*72=144MHz
FOUT=FIN*NF/NR/NO=4MHz*96/2/1=144MHz

Dividers
USB: /3 by CLKDIV     -> 48MHz
HCLK_N: /2 by CLKDIV  -> 72MHz
    used to be CLKSEL0_HCLS_S_PLL_DIV2 but conflicting definitions in the manual/library

CLOCK INIT PROCESS:
write all other registers, *1 to enable HXT, wait for stable clock, *2 to switch to HXT, *3 to disable HIRC

This leads to register config:
PWRCON *1:              0x00000035 //Instant powerdown with PWR_DOWN_EN, Interrupts enabled, wait for stable clock, HIRC on, HXT on
AHBCLK:                 0x00000004 //ISP
APBCLK:                 0x08010200 //USBD, UART0, I2C1
CLKSEL0 *2:             0x00000001 //STCLK_S=HXT/1,HCLK_S=PLL/2 		!only write to after source and destination clock is stable
PWRCON *3:              0x00000031 //Instant powerdown with PWR_DOWN_EN, Interrupts enabled, wait for stable clock, HXT on (HIRC off)
CLKSEL1:                0x5122227A //all periphial clocks on HCLK
CLKSEL2:                0x0002000A //all periphial clocks on HCLK
CLKDIV:                 0x00000020 //USB/3
PLLCON:                 0x00000046 //Setup for 144MHz see above
FRQDIV:                 0x00000000 //Frequency divider and clock output pin disabled
APBDIV:                 0x00000000 //default, no divider
