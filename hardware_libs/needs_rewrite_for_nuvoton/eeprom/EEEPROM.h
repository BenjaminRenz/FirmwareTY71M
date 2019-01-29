

#ifndef EEEPROM_h
#define EEEPROM_h

#include <Arduino.h>
#include <Wire.h> //I2C library

class EEEPROM
{
	private:
	public:
		EEEPROM();
		void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data );
		void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length );
		byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress );
		void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length );
};

#endif
