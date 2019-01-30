//Original Code from https://github.com/futureshocked/EEEPROM
//TODO Still need modification
//TODO #include <I2C.h>
#include <EEEPROM.h>


void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ){
  int rdata = data;
  i2c_beginTransmission(deviceaddress);
  i2c_write((int)(eeaddress >> 8)); // MSB
  i2c_write((int)(eeaddress & 0xFF)); // LSB
  i2c_write(rdata);
  i2c_endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, unsigned char* data, unsigned char length ){
  i2c_beginTransmission(deviceaddress);
  i2c_write((int)(eeaddresspage >> 8)); // MSB
  i2c_write((int)(eeaddresspage & 0xFF)); // LSB
  byte c;
  for ( c = 0; c < length; c++)
    i2c_write(data[c]);
  i2c_endTransmission();
  
}
unsigned char i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ){
  byte rdata = 0xFF;
  i2c_beginTransmission(deviceaddress);
  i2c_write((int)(eeaddress >> 8)); // MSB
  i2c_write((int)(eeaddress & 0xFF)); // LSB
  i2c_endTransmission();
  i2c_requestFrom(deviceaddress,1);
  if (i2c_available()) rdata = i2c_read();
  return rdata;
  
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ){
  i2c_beginTransmission(deviceaddress);
  i2c_write((int)(eeaddress >> 8)); // MSB
  i2c_write((int)(eeaddress & 0xFF)); // LSB
  i2c_endTransmission();
  i2c_requestFrom(deviceaddress,length);
  int c = 0;
  for ( c = 0; c < length; c++ )
    if (i2c_available()) buffer[c] = i2c_read();
}
