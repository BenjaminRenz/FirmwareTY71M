0) Requiered Hardware
-StLink Programmer or other programmer supported by openocd
-Windows Pc
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
    Putty: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html

4) run START_debugger.bat
    This will open a command prompt and launch openocd with the correct configuration
    
4b) if it says something like
        Error: init mode failed (unable to connect to the target)
        in procedure 'init'
        int procedure 'ocd_bouncer'
    You have probably misswired the stlink, like connecting the data pin accidentaly to SWIM and not SWDIO (thats the way I found out about this problem).

5) Open Putty, switch connection type to Telnet, enter localhost in Host Name field, enter Port 4444 and click open

6) Putty should display Open On-Chip Debugger, enter 
  reset halt
    to stop the microcontroller.
    The keyboard's backlight should have gone out. Sometimes Polling target NuMicro.cpu failed, trying to reexamine appears, in this situation enter init and reset halt and if this is not helping connect the RST pin of the keyboard to the other GND pin of the StLink for 1 sec and restart openocd.

7) Check if your chip is security locked (flash info 0 will return wrong information regarding flash lock state, ignore that output)
  numicro read_isp 0x00300000
    Result will be something like 0x00300000: 0x78bfffb8
    Take a look at the bit at 0x00000002 or (1<<1), so the second leftmost bit. It if is 0 your flash is security locked and can't be read by the isp.
    It it is 1 please let me know, you should be able to backup your firmware, do that first before proceeding any further.

8) Compile the firmware by opening CodeBlocks according to HOWTOSTARTUP.md and compile with !!!Important!!! "Just Compile" configuration selected
		You should have a new firmware.bin file in the firmware_c/out folder
		
9) WARNING! will wipe the existing firmware on your chip WARNING! As of now there is no way going back
		Do this for unlocking the read/write protection if present
  numicro chip_erase
    returns: numicro chip_erase complete
    All the data on the chip will be gone,  numicro read_isp 0x00300000 will return 0xffffffff
		
10) prepare for flashing
  init
  reset init
     
11) flash image
  flash write_image ./firmware_c/out/firmware.bin

12) verfy flash
  flash verify_bank 0 ./firmware_c/out/firmware.bin
  
13)
  reset run


	
	
EXTRA
	Interesting commands of openocd:
    flash info 0 (displays info of flash bank 0) (incorrect lock info!)
    flash banks (displays all four flash banks and their size)
    mdw 0x50000100 (reads memory from address 0x50000100) (does not work for 0x00300000)
    flash erase_check 0 (checks if memory at bank 0 is erazed)
