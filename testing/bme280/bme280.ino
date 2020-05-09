/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

unsigned long delayTime;

void setup() {
    Serial.begin(9600);

    // default settings
    bme.begin(0x76);
    
    Serial.println("Temp,pressure,humidity");
    delayTime = 100;
}


void loop() { 
    printValues();
    delay(delayTime);
}


void printValues() {
    Serial.print(bme.readTemperature());
    Serial.print(",");
    Serial.print(bme.readPressure() / 1000.0F);
    Serial.print(",");
    Serial.print(bme.readHumidity());
    Serial.println();
}
