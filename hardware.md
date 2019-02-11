Hardware of Drevo 71/72 key variant:
---Bluetooth 4.0 model also called V1.3---
-NUC123LD4AN (labeled as M103CB 171301)
-EEPROM 24C256
-CSR1010 Bluetooth 4.0 module

Default Pin input value reading (only rightmost bits are relevant)
	 battery removed	battery installed, power from usb		power from battery
PA 0x0000ec00 			0x0000ac00		 											0x0000ec00
PB 0x0000f7ff 			0x0000f7ff     											0x0000b7ff
PC 0x00003f3f 			0x00003f3f     											0x00003f3f
PD 0x00000f3f 			0x00000f3f     											0x00000f3f
PF 0x00000006 			0x00000006     											0x0000000e

Conclusions:
->all diode side pins are pulled up to high logic level
->input PF3  (power source?) changes from low to high when powered by usb
->input PA14 (battery charging) changes from low to high when charging battery from usb
->input PB14  (power source?) changes from low to high when powered by usb
->output PA12 (backlight) (backlight on for high, backlight off for low)

---Bluetooth 3.0 model also called V1.2---
-probably NUC123LD4AN (labeled as M103CB 171301)
-EEPROM 24C256
-Broadcom® BCM20730 Bluetooth 3.0 module