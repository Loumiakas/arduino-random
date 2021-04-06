#include <Wire.h>


#define I2C_ADDR        0b01010000
#define EEPROM_SIZE     256 

void setup()
{
    //
    //  Initialize Serial and Wire.
    //
    Serial.begin(9600);
    Wire.begin();
    printWelcomeMessage(); 
    //
    //  Create a byte array to store all the data.
    //
    byte eeprom_data[EEPROM_SIZE];
    //
    //  Iterate through EEPROM memory and extract the bytes.
    //
    for(int i=0; i<sizeof(eeprom_data); ++i)
    {
        eeprom_data[i] = readI2CAddr(i);
    }
    //
    //  Print data grid in hex format.
    //
    printData(eeprom_data, sizeof(eeprom_data));
}

void printWelcomeMessage()
{
    Serial.println("==== Atmel AT24C02 I2C EEPROM data dumper ====");
    Serial.print("I2C Address:\t bin=");
    Serial.print(I2C_ADDR, BIN);
    Serial.print(" (0x");
    Serial.print(I2C_ADDR, HEX);
    Serial.println(")");
    Serial.print("EEPROM Capacity: ");
    Serial.print(EEPROM_SIZE);
    Serial.println(" bytes");
}

void printData(byte *data, unsigned int len)
{
    for(int i=0; i<len; ++i)
    {
        if( i % 16 == 0 )
            Serial.println(" ");
        if (data[i] <= 0xF)
            Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println("\n");
    Serial.println("Data dump finished...");
}

byte readI2CAddr(unsigned int addr)
{
    byte data = NULL;
    //
    //  Perform dummy write to move pointer to specified location.
    //
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(addr);
    Wire.endTransmission();
    //
    //  Attempt to read one byte of data at specified location.
    //
    Wire.requestFrom(I2C_ADDR, 1);
    delay(1);
    //
    //  If the data is available, return it
    //
    if (Wire.available())
    {
        data = Wire.read();
    }
    return data;
}

void loop()
{
    //
    //  Not used.
    //
}
