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

void writeEEPROM(int deviceaddress, unsigned int eeaddress, char* data) 
{
  // Uses Page Write for 24LC256
  // Allows for 64 byte page boundary
  // Splits string into max 16 byte writes
  unsigned char i=0, counter=0;
  unsigned int  address;
  unsigned int  page_space;
  unsigned int  page=0;
  unsigned int  num_writes;
  unsigned int  data_len=0;
  unsigned char first_write_size;
  unsigned char last_write_size;  
  unsigned char write_size;  
  
  // Calculate length of data
  do{ data_len++; } while(data[data_len]);   
   
  // Calculate space available in first page
  page_space = int(((eeaddress/64) + 1)*64)-eeaddress;

  // Calculate first write size
  if (page_space>16){
     first_write_size=page_space-((page_space/16)*16);
     if (first_write_size==0) first_write_size=16;
  }   
  else 
     first_write_size=page_space; 
    
  // calculate size of last write  
  if (data_len>first_write_size) 
     last_write_size = (data_len-first_write_size)%16;   
  
  // Calculate how many writes we need
  if (data_len>first_write_size)
     num_writes = ((data_len-first_write_size)/16)+2;
  else
     num_writes = 1;  
     
  i=0;   
  address=eeaddress;
  for(page=0;page<num_writes;page++) 
  {
     if(page==0) write_size=first_write_size;
     else if(page==(num_writes-1)) write_size=last_write_size;
     else write_size=16;
  
     Wire.beginTransmission(deviceaddress);
     Wire.write((int)((address) >> 8));   // MSB
     Wire.write((int)((address) & 0xFF)); // LSB
     counter=0;
     do{ 
        Wire.write((byte) data[i]);
        i++;
        counter++;
     } while((data[i]) && (counter<write_size));  
     Wire.endTransmission();
     address+=write_size;   // Increment address for next write
     
     delay(6);  // needs 5ms for page write
  }
}
 
void readEEPROM(int deviceaddress, unsigned int eeaddress,  
                 unsigned char* data, unsigned int num_chars) 
{
  unsigned char i=0;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(deviceaddress,num_chars);
 
  while(Wire.available()) data[i++] = Wire.read();

}

void get_mock_data(char *buf, int num) {
    char temp_mock[] = "20081708560010000000010000";
    char tmp[4];

    char ibuf[4];
    if (num < 10) {
        strcpy(tmp, "000");
        itoa(num, ibuf, 10);
        strcat(tmp, ibuf);
    } else if (num < 100) {
        strcpy(tmp, "00");
        itoa(num, ibuf, 10);
        strcat(tmp, ibuf);
    } else if (num < 1000) {
        strcpy(tmp, "0");
        itoa(num, ibuf, 10);
        strcat(tmp, ibuf);
    } else {
        //itoa(num, ibuf, 10);
        strcpy(tmp, "1000");
    }
    strcat(temp_mock, tmp);
    strcpy(buf, temp_mock);
}

void setup()
{
    char somedata[] = "this is data from the eeprom"; // data to write
    char mockdata[] = "20081708560010000000010000000"; // 30 chars of gps info
    char tmp[30];
    unsigned int start_mem_w = 0;
    unsigned int start_mem_u = 4;
    unsigned int start_w = 8;

    char mem_w[] = "0001";
    char mem_u[] = "0002";

    Wire.begin(); // initialise the connection
    Serial.begin(9600);
    //i2c_eeprom_write_page(0x50, 0, (byte *)mockdata, sizeof(mockdata)); // write to EEPROM
    //i2c_eeprom_write_byte(0x50, 30, tmp[0]);

    delay(100); //add a small delay

    Serial.println("Start write position...");
    Serial.print("Mem W: ");
    Serial.println(mem_w);
    Serial.print("Mem U: ");
    Serial.println(mem_u);
    writeEEPROM(EEPROM1, start_mem_w, mem_w);
    writeEEPROM(EEPROM1, start_mem_u, mem_u);

    char tmp4[4];
    readEEPROM(EEPROM1, start_mem_w, tmp4, 4);
    Serial.print("RMem W: ");
    Serial.println(tmp4);
    readEEPROM(EEPROM1, start_mem_u, tmp4, 4);
    Serial.print("RMem U: ");
    Serial.println(mem_u);
    

    char buf[5];
    Serial.println("Start write...");
    for (int i = 0; i < 10; i++) {
        get_mock_data(tmp,i);
        writeEEPROM(EEPROM1, (i * 30) + start_w, tmp);
        //i2c_eeprom_write_page(0x50, (i * sizeof(tmp)), (byte *)tmp, sizeof(tmp)); // write to EEPROM
        Serial.print("Write: ");
        Serial.print(tmp);
        Serial.print(" [");
        Serial.print(strlen(tmp));
        Serial.println("]");
    }
    
    Serial.println("Memory written");

    strcpy(tmp, "");
   for (int j = 0; j < 10; j++) {
        //i2c_eeprom_read_buffer(0x50, (j * 30), tmp, 30);
        readEEPROM(EEPROM1, (j * 30) + start_w, tmp, 30);
        Serial.print("Read: ");
        Serial.print(tmp);
        Serial.print(" [");
        Serial.print(strlen(tmp));
        Serial.println("]");
    }
}

void loop()
{
    /*
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
    */
}

