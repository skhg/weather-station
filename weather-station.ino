/**
 * Network Configuration
 */
#define IP_1 192
#define IP_2 168
#define IP_3 178
#define IP_4 29
#define IP_PORT 8080
#define WIFI_CONNECT_TIMEOUT_SECONDS 30

/**
 * System properties
 */
#define REBOOT_DELAY_MS 1000

/**
 * Pin placements
 */
#define ANALOGUE_IN A0
#define REF_3V3 16 // D0
#define SDS_DUST_TX D3
#define SDS_DUST_RX D4
#define MULTIPLEX_S0 14 // D5
#define MULTIPLEX_S1 12 // D6
#define MULTIPLEX_S2 13 // D7
#define MULTIPLEX_S3 15 // D8

/** 
 *  Application below
 */
 
#include <home_wifi.h>
#include <ESP8266WiFi.h>
#include <Adafruit_BME280.h>
#include "SdsDustSensor.h"
#include <ArduinoJson.h>


/**
 * Instruments
 */
Adafruit_BME280 bme;
SdsDustSensor sds(SDS_DUST_RX, SDS_DUST_TX);

/**
 * Network messages
 */
const int dataFieldsCapacity = JSON_OBJECT_SIZE(8);
StaticJsonDocument<dataFieldsCapacity> dataJson;
WiFiClient client;
IPAddress server(IP_1,IP_2,IP_3,IP_4);

/**
 * Runtime parameters
 */
unsigned long previousTime = 0;
float ultravioletDarknessVoltage = 1.07;
float ultravioletMaxVoltage = 2.8;
float maxUltravioletIntensityLevelAtLocation = 8.0;
int sdsInhalationSeconds = 30;

void setup(){
  Serial.begin(9600);

  Serial.println("Booting");
  delay(REBOOT_DELAY_MS); //Allow a moment for some sensors to initialise

  previousTime = millis();
  
  connectToWifi();
  setupMultiplexer();
  setupInstruments();
}

void loop(){
  unsigned long currentTime = millis();

  double temperature;
  double humidity;
  double pressure;

  getBme280data(&temperature, &pressure, &humidity);

  double rainfallLevel;
  getRainfallData(&rainfallLevel);

  double uvIntensity;
  getUvSensorData(&uvIntensity);

  double pm25level;
  double pm10level;
  getParticulateData(&pm25level, &pm10level);

}

void setActiveMultiplexerChannel(int channelNum){
  digitalWrite(MULTIPLEX_S0, bitRead(channelNum, 0));
  digitalWrite(MULTIPLEX_S1, bitRead(channelNum, 1));
  digitalWrite(MULTIPLEX_S2, bitRead(channelNum, 2));
  digitalWrite(MULTIPLEX_S3, bitRead(channelNum, 3));
}

void setupMultiplexer(){
  pinMode(MULTIPLEX_S0, OUTPUT);
  pinMode(MULTIPLEX_S1, OUTPUT);
  pinMode(MULTIPLEX_S2, OUTPUT);
  pinMode(MULTIPLEX_S3, OUTPUT);
}

void setupInstruments(){
  // Temp/humidity/pressure sensor
  bme.begin(0x76);

  // Dust sensor
  sds.begin();
  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setQueryReportingMode().toString()); // ensures sensor is in 'query' reporting mode
}

void getBme280data(double*temperature, double*pressure, double*humidity){
  *temperature = bme.readTemperature();
  *pressure = bme.readPressure();
  *humidity = bme.readHumidity();
}

void getRainfallData(double*rainfallPercentage){
  setActiveMultiplexerChannel(1);
  int rawWetness = 1024 - analogRead(ANALOGUE_IN);
  
  double rainPercent = mapDouble((double)rawWetness, 0.0, 1024.0, 0.0, 100.0);
  *rainfallPercentage = rainPercent;
}

void getUvSensorData(double*uvIntensity){
  setActiveMultiplexerChannel(0);
  int uvLevel = averageAnalogueRead(ANALOGUE_IN);
  int refLevel = averageAnalogueRead(REF_3V3);
  double outputVoltage = 3.3 / refLevel * uvLevel;
  double mapped = mapDouble(outputVoltage, ultravioletDarknessVoltage, ultravioletMaxVoltage, 0.0, maxUltravioletIntensityLevelAtLocation);

  *uvIntensity = mapped;
}

void getParticulateData(double*pm25, double*pm10){
  sds.wakeup();
  delay(sdsInhalationSeconds * 1000);

  PmResult pm = sds.queryPm();
  if (pm.isOk()) {
    *pm25 = pm.pm25;
    *pm10 = pm.pm10;
  } else {
    Serial.print("Could not read values from sensor, reason: ");
    Serial.println(pm.statusToString());
  }

  WorkingStateResult state = sds.sleep();
  if (state.isWorking()) {
    Serial.println("Problem with sleeping the sensor.");
  }
}

void getWindSpeed(double*windSpeed){
  setActiveMultiplexerChannel(2);

  //todo get wind speed
  *windSpeed = 2.2;
}

void sendData(double temperature, double humidity, double pressure, double uvIntensity, double rainfallLevel, double windSpeed, double pm25level, double pm10level){
  dataJson["temperatureC"] = temperature;
  dataJson["airPressurePa"] = pressure;
  dataJson["humidityPercentage"] = humidity;
  dataJson["uvIntensity"] = uvIntensity;
  dataJson["rainfall"] = rainfallLevel;
  dataJson["windSpeed"] = windSpeed;
  dataJson["pm25level"] = pm25level;
  dataJson["pm10level"] = pm10level;

  String dataJsonOutput;
  serializeJson(dataJson, dataJsonOutput);

  //todo use the nicer http library
  
  if (client.connect(server, IP_PORT)) {      
    // Make a HTTP request:
    client.println("POST /weather-station/balcony HTTP/1.0");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(dataJsonOutput.length());
    client.println();
    client.println(dataJsonOutput);
    client.println();
    client.stop();
  }else{
    Serial.println("ERROR: Failed to connect");
    reboot();
  }
}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogueRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}

/**
 * System management functions
 */

void reboot(){
  delay(REBOOT_DELAY_MS);
  Serial.println("Rebooting...");
  ESP.restart();
}

void connectToWifi(){
  String wifiConnectionInfo = "Connecting to WiFi";
  
  if(WiFi.status() == WL_CONNECTED){
    return;  
  }
  
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int connectAttempts = 0;
  int connectRetryInterval = 500;
  int rebootCountdown = WIFI_CONNECT_TIMEOUT_SECONDS * 1000;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(connectRetryInterval);
    
    Serial.print(".");

    rebootCountdown = rebootCountdown - connectRetryInterval;
    
    if(rebootCountdown < 0) {
      reboot();
    }
  }
  
  Serial.println("");
  Serial.println("WiFi connected"); 
}
