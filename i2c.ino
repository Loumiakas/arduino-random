#include <Wire.h>
/*
 *  Wire library  expects 7-bit I2C device address. In the specification, it
 *  is stated that the device address will always start with first four bytes
 *  as being 1010 followed by value of hard-wired address pins  A2, A1 and A0.
 *  These pins are set to low on my setup, so they are set to 000
 *  respectively. Last bit is used for specifying read/write operation, and
 *  becuase of that, wire library expects the address bits shifted once to the 
 *  right.
 */
#define I2C_ADDR        0b01010000
/*
 *  AT24C02 is labelled as having 2K (256*8) capacity.
 */
#define EEPROM_SIZE     256 

void setup()
{
    //
    //  Initialize Serial and Wire.
    //
    Serial.begin(9600);
    Wire.begin();
    //
    //  Check connection status, if device is not found/unavailable, exit with
    //  an error message.
    //
    int conn_stat = getConnectionStatus();
    if ( conn_stat == 0 )
    {
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
    else
    {
        Serial.println("Chip connection failed...");
    }
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

void printData(const byte *data, const unsigned int len)
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

byte readI2CAddr(const unsigned int addr)
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
    //
    //  If the data is available, return it.
    //
    if (Wire.available())
    {
        data = Wire.read();
    }
    return data;
}

int writeI2CAddr(const unsigned int addr, const byte val)
{
    //
    //  Perform write of address value, followed by the value.
    //
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(addr);
    Wire.write(val);
    int stat =  Wire.endTransmission();
    //
    //  Put a small delay and return status.
    //
    delay(8);
    return stat;
}

int getConnectionStatus()
{
    Wire.beginTransmission(I2C_ADDR);
    return Wire.endTransmission();
}

void loop()
{
    //
    //  Not used.
    //
}
