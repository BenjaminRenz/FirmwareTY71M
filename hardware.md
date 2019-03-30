Hardware of Drevo 71/72 key variant:

SECTION MICROCONRTOLLER	
	NUC123LD4AN (labeled as M103CB 171301)

SECTION EEPROM
	EEPROM 24C256 -> over i2c1 @ 3.3v supply voltage

SECTION Bluetooth
	Bluetooth 4.0 model also called V1.3
		CSR1010 Bluetooth 4.0 module -> over uart
	Bluetooth 3.0 model also called V1.2
		Broadcom® BCM20730 Bluetooth 3.0 module
		
SECTION IO
	SUBSECTION RGB BACKLIGHT (DONE)
		Shift register chain order LED_SELECT (8bit)->RED(9bit)->BLUE(9bit)->GREEN(9bit)
		
		LED_SELECT (border in diagram) light_green ,yellow,orange,dark_green,light_blue,tourquise,pink,red
		
		RED BLUE GREEN: (colored boxes in diagram)  yellow,green,dark_blue,pink,light_blue,red,orange,black,grey

		LED shift off bit
		set output to high (shift_data)
		mww 0x50004088 0x0000FFFF
		toogle shift clock on (8x)
		mww 0x50004048 0x0000FBFF
		mww 0x50004048 0x0000FFFF
		toogle rclk
		mww 0x50004088 0x0000FFFE
		mww 0x50004088 0x0000FFFF

		LED shift on bit
		set output to low (shift_data)
		mww 0x50004088 0x0000FFFD
		toogle shift clock on (n-times)
		mww 0x50004048 0x0000FBFF
		mww 0x50004048 0x0000FFFF
		toogle rclk (to display values on the output of the shift register(s))
		mww 0x50004088 0x0000FFFC
		mww 0x50004088 0x0000FFFD
	
	SUBSECTION KEY MATRIX (WIP)
		The non diode side is pulled up to VCC, so the diode side (rows) needs to get pulled low one after each other.
		After that the columns are sampled.
		Row Layout (output, pulled low to enable)
		??????????????????????????????????????????????????????
		? 0  0  0  0  0  0  0  0  5  5  5  5  5   5  5  5  8 ?
		? 1   1  1  1  1  1  1  1  6  6  6  6  6  6  6  6  8 ?
		?  2   2  2  2  2  2  2  2  7  7  7  7   7           ?
		? 3  8  3  3  3  3  3  3  3  8  8  8    8       8    ?
		? 4  4  4  ---------4--------  4  4  4   4   7  7  7 ?
		??????????????????????????????????????????????????????
		Column Layout (input, low when pressed)
		??????????????????????????????????????????????????????
		? 0  1  2  3  4  5  6  7  0  1  2  3  4   5  6  7  4 ?
		? 0   1  2  3  4  5  6  7  0  1  2  3  4  5  6  7  5 ?
		?  0   1  2  3  4  5  6  7  0  1  2  3   4           ?
		? 0  7  1  2  3  4  5  6  3  0  1  2    3       6    ?
		? 0  1  2  ---------3--------  4  5  6   7   5  6  7 ?
		??????????????????????????????????????????????????????
		
	
	SUBSECTION Additional IO
		Default Pin input value reading (only rightmost bits are relevant)
			battery removed	battery installed, power from usb		power from battery
			PA 0x0000ec00 			0x0000ac00		 											0x0000ec00
			PB 0x0000f7ff 			0x0000f7ff     											0x0000b7ff
			PC 0x00003f3f 			0x00003f3f     											0x00003f3f
			PD 0x00000f3f 			0x00000f3f     											0x00000f3f
			PF 0x00000006 			0x00000006     											0x0000000e

		Conclusions:
		->input PF3  (power source?) changes from low to high when powered by usb
		->input PA14 (battery charging) changes from low to high when charging battery from usb
		->input PB14  (power source?) changes from low to high when powered by usb
		->output PA12 (backlight) (backlight on for high, backlight off for low)

