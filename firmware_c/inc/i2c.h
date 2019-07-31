#include "custom.h"
#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED
void I2C1_IRQHandler(void){

}

void I2C1_init(){
    //clock should be already enabeled

}
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data );
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, unsigned char* data, unsigned char length );
unsigned char i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress );
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length );



#define EEPROM_read_address 0xA0 //TODO test if correct
#define EEPROM_write_address (EEPROM_read_address+1) //TODO test if correct

void EEEPROM::i2c_eeprom_write_byte(, unsigned int eeaddress, byte data ) {
  int rdata = data;
  Wire.beginTransmission(EEPROM_address);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(rdata);
  Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void eeprom_write_page(uint16_t memory_address,uint8_t* data,uint8_t numberOFbytes){
    Wire.beginTransmission(deviceaddress);
  I2C_write((uint8_t)(memory_address >> 8));   // MSB
  I2C_write((uint8_t)(memory_address & 0xFF)); // LSB
  for (uint8_t currentByte = 0; currentByte < numberOFbytes; currentByte++){
    I2C_write(data[currentByte]);
  }
  Wire.endTransmission();
}

void I2C_write(uint8_t data){

}
uint8_t eeprom_read_byte(uint16_t memory_address) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  I2C_write((uint8_t)(memory_address >> 8));   // MSB
  I2C_write((uint8_t)(memory_address & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void EEEPROM::i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,length);
  int c = 0;
  for ( c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}

#endif // I2C_H_INCLUDED
