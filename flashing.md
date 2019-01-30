0) Requiered Hardware
-StLink Programmer or other programmer supported by openocd
-Windows Pc
-ty71 keyboard (disconnect lithium battery for savety)

1) Connect the pins of P3 as the following:
    | | -> GND of ST-Link
    | | -> RST (*)
    | | -> NotConnected (Power keyboard with USB instead)
    | | -> SWDIO
    | | -> SWCLK
  see pictures for reference "keyboard cables.jpg" and "stlink cables.jpg"
    
    
2) Connect the micro USB to the keyboard

3) Install:
    StLinkDriver: https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html (bottom of page, you need to be registered)
    OpenOCD: http://gnutoolchains.com/arm-eabi/openocd/
    Putty: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html

4) extract OpenOCD archive and copy the content of bin folder into share/openocd/scripts/ (3 files including openocd.exe)

5) go into the scripts folder if you are not already there and

6) Shift+Rightclick in openocd and "Open command window here"

7) Enter openocd.exe -f interface\stlink.cfg -f target\numicro.cfg

7b) if it says something like
        Error: init mode failed (unable to connect to the target)
        in procedure 'init'
        int procedure 'ocd_bouncer'
    You have probably misswired the stlink, like connecting the data pin acciedentaly to SWIM and not SWDIO (thats the way I found out about this problem).

8) Open Putty, switch connection type to Telnet, enter localhost in Host Name field, enter Port 4444 and click open

9) Putty should display Open On-Chip Debugger, enter reset halt. The keyboard's backlight should have gone out. Sometimes Polling target NuMicro.cpu failed, trying to reexamine appears, in this situation enter init and reset halt and if this is not helping connect the RST pin of the keyboard to the other GND pin of the StLink for 1 sec and restart openocd.

10) TODO!
unlock write protection
erase chip
upload the custom firmware



Interesting commands:
    flash info 0 (displays info of flash bank 0)
    flash banks (displays all four flash banks and their size)
    mdw 0x50000100 (reads memory from addres 0x50000100)
    numicro read_isp 0x00300000 (seems to be requiered to read flash config0 register (0x00300000) which locks flash)
