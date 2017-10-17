/*
  *  Use the I2C bus with EEPROM 24LC64
  *  Sketch:    eeprom.ino
  *
  *  Author: hkhijhe
  *  Date: 01/10/2010
  *
  *
  */

#include <Wire.h>

#define EEPROM1 0x50

// Data - 200817085600100000000100000000 (30 Chars, First 12 digits is date-time, then 9 digits is lat and last 9 digits is lng
// EEPROM size is 32 KByte that will keep data around 1000 records
// for every 10 sec (6 rec per min) can keep data around 2.7 hours
// for every 15 sec (4 rec per min) can keep data around 4.1 hours
// for every 30 sec (2 rec per min) can keep data around 8.3 hours
// for every 1 min can keep data around 16 hours

void write_trx_start_addr( unsigned int num_trx) {
    // num_trx should not over 1000 **
}

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
// Wire lib buffer 32 bytes - first 2 bytes for address and 30 bytes for payload
// Max trx to write is 1000 for 30 bytes per trx
// first 4 bytes (0-3) will keep address to start write
// and second 4 bytes (4-7) will keep address to start upload to server
// then address to start to write trx should be 8
// actual address calculate by (number of trx * size of trx (30)) + 8
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddresspage >> 8)); // MSB
    Wire.write((int)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
        Wire.write(data[c]);
    Wire.endTransmission();
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    /*int ad1 = (int)(eeaddress >> 8);
    int ad2 = (int)(eeaddress & 0xFF);

    Serial.println("=== Debug ===");
    Serial.println(ad1);
    Serial.println(ad2);
    Serial.println("==== End ====");*/
    
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.read();
    return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
        if (Wire.available()) buffer[c] = Wire.read();
}




void setup()
{
    char somedata[] = "this is data from the eeprom"; // data to write
    char mockdata[] = "20081708560010000000010000000"; // 30 chars of gps info
    char tmp[] = "0";
    Wire.begin(); // initialise the connection
    Serial.begin(9600);
    //i2c_eeprom_write_page(0x50, 0, (byte *)mockdata, sizeof(mockdata)); // write to EEPROM
    //i2c_eeprom_write_byte(0x50, 30, tmp[0]);

    delay(100); //add a small delay

    Serial.println("Memory written");
}

void loop()
{
    int addr=0; //first address
    byte b = i2c_eeprom_read_byte(EEPROM1, 0); // access the first address from the memory

    while (b!=0)
    {
        Serial.print((char)b); //print content to serial port
        addr++; //increase address
        b = i2c_eeprom_read_byte(EEPROM1, addr); //access an address from the memory
    }
    Serial.println(" ");
    Serial.print("Total characters: ");
    Serial.println(addr+1);
    delay(2000);
}

