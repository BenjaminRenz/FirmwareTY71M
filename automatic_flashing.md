0) Requiered Hardware
-StLink Programmer or other programmer supported by openocd
-Windows PC, if you are planning to use the existing setup
-ty71 keyboard (disconnect lithium battery for safety)

1) Connect the pins of P3 as the following:
    | | -> GND of ST-Link
    | | -> RST (*)
    | | -> NotConnected (Power keyboard with USB instead)
    | | -> SWDIO
    | | -> SWCLK
  see pictures for reference "keyboard cables.jpg" and "stlink cables.jpg"
    
    
2) Connect the micro USB to the keyboard for power

3) Install:
    StLinkDriver: https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html (bottom of page, you need to be registered)
