0) Required Hardware
-StLink Programmer or other programmer supported by Openocd
-Windows PC, if you are planning to use the existing setup
-ty71 keyboard (disconnect lithium battery for safety)

1) Connect the pins of P3 as the following:
    | | -> GND of ST-Link
    | | -> RST (*)
    | | -> NotConnected (Power keyboard with USB instead)
    | | -> SWDIO
    | | -> SWCLK
  see pictures for reference "keyboard cables.jpg" and "stlink cables.jpg"
    
    
2) Connect the keyboard with a micro USB to your pc to power it.

3) Install:
	Even though Windows 10 provides it's own driver I'm getting an error "LIBUSB_ERROR_NOT_SUPPORTED" with openocd. It works with the official driver from st though:
    StLinkDriver: https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html (bottom of page, you need to be registered)

4) Start Codeblocks with the "START_codeblocks_with_gdb.bat" file, tweak the firmware to your liking, then recompile it with the "blue arrows in circle" icon.
   Select "Compile and Upload" from the dropdown next to the compile icon, then hit the red play button which will start the debugger and upload the firmware to your keyboard.