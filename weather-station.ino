/**
 * Copyright 2020 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

/**
 * Installation-specific settings
 */

// Transparency (0.0 to 1.0) of the glass cover on the UV sensor
#define UV_SENSOR_CAP_TRANSPARENCY 0.9

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
 * Network hostname for the system
 */

const String HOST_NAME = "weather-station";

/**
 * System properties
 */
#define REBOOT_DELAY_MS 1000

/**
 * Pin placements
 */
#define ANALOGUE_IN A0
#define REF_3V3 16  // D0
#define SDS_DUST_TX D3
#define SDS_DUST_RX D4
#define MULTIPLEX_S0 14  // D5
#define MULTIPLEX_S1 12  // D6
#define MULTIPLEX_S2 13  // D7
#define MULTIPLEX_S3 15  // D8

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
IPAddress server(IP_1, IP_2, IP_3, IP_4);

/**
 * Runtime parameters
 */
uint64_t previousTime = 0;
float ultravioletDarknessVoltage = 1.07;
float ultravioletMaxVoltage = 2.8;
float maxUltravioletIntensityLevelAtLocation = 8.0;
int sdsInhalationSeconds = 30;
int windMeasurementSeconds = 10;
int cycleIntervalSeconds = 60;

void setup() {
  Serial.begin(9600);

  Serial.println("Booting");
  delay(REBOOT_DELAY_MS);  // Allow a moment for some sensors to initialise

  previousTime = millis();

  connectToWifi();
  setupMultiplexer();
  setupInstruments();

  Serial.println("Setup complete!");
  Serial.println("");
}

void loop() {
  double temperature;
  double humidity;
  double pressure;

  getBme280data(&temperature, &pressure, &humidity);

  double rainfallLevel;
  getRainfallData(&rainfallLevel);

  double uvIntensity;
  getUvSensorData(&uvIntensity);

  double windSpeed;
  getWindSpeedKmPerHr(&windSpeed);

  double pm25level;
  double pm10level;
  getParticulateData(&pm25level, &pm10level);

  sendData(temperature, humidity, pressure, uvIntensity, rainfallLevel,
  windSpeed, pm25level, pm10level);

  sleepUntilNext();
}

void sleepUntilNext() {
  Serial.println("Cycle complete.");
  Serial.println("---");
  Serial.println("Sleeping for " + String(cycleIntervalSeconds) +
  " seconds...");
  Serial.println("");

  delay(cycleIntervalSeconds * 1000);
}

void setActiveMultiplexerChannel(int channelNum) {
  digitalWrite(MULTIPLEX_S0, bitRead(channelNum, 0));
  digitalWrite(MULTIPLEX_S1, bitRead(channelNum, 1));
  digitalWrite(MULTIPLEX_S2, bitRead(channelNum, 2));
  digitalWrite(MULTIPLEX_S3, bitRead(channelNum, 3));
}

void setupMultiplexer() {
  pinMode(MULTIPLEX_S0, OUTPUT);
  pinMode(MULTIPLEX_S1, OUTPUT);
  pinMode(MULTIPLEX_S2, OUTPUT);
  pinMode(MULTIPLEX_S3, OUTPUT);
}

void setupInstruments() {
  Serial.println("Setup BME280...");
  // Temp/humidity/pressure sensor
  bme.begin(0x76);
  Serial.println("");

  Serial.println("Setup SDS011...");
  // Dust sensor
  sds.begin();
  Serial.println(sds.queryFirmwareVersion().toString());
  Serial.println(sds.setQueryReportingMode().toString());
  sds.sleep();
  Serial.println("");
}

void getBme280data(double*temperature, double*pressure, double*humidity) {
  double liveTemp = bme.readTemperature();
  double livePressure = bme.readPressure();
  double liveHumidity = bme.readHumidity();

  Serial.println("BME280");
  Serial.println("---");
  Serial.println("Temperature: " + String(liveTemp));
  Serial.println("Pressure: " + String(livePressure));
  Serial.println("Humidity: " + String(liveHumidity));
  Serial.println("");

  *temperature = liveTemp;
  *pressure = livePressure;
  *humidity = liveHumidity;
}

void getRainfallData(double*rainfallPercentage) {
  setActiveMultiplexerChannel(1);
  int sensorReading = analogRead(ANALOGUE_IN);
  int maxValue = 1024;
  int rawWetness = maxValue - sensorReading;
  double rainPercent = mapDouble(static_cast<double>(rawWetness), 0.0, 1024.0,
  0.0, 100.0);

  Serial.println("Rainfall Sensor");
  Serial.println("---");
  Serial.println("Sensor reading: " + String(sensorReading));
  Serial.println("Wetness: " + String(rawWetness) + " / " + String(maxValue));
  Serial.println("Wetness %: " + String(rainPercent));
  Serial.println("");

  *rainfallPercentage = rainPercent;
}

void getUvSensorData(double*uvIntensity) {
  Serial.println("ML8511");
  Serial.println("---");
  setActiveMultiplexerChannel(0);
  int uvLevel = averageAnalogueRead(ANALOGUE_IN);
  int refLevel = averageAnalogueRead(REF_3V3);
  double outputVoltage = 3.3 / refLevel * uvLevel;
  double mapped = mapDouble(outputVoltage, ultravioletDarknessVoltage,
  ultravioletMaxVoltage, 0.0, maxUltravioletIntensityLevelAtLocation);
  double adjustedIntensity = mapped / UV_SENSOR_CAP_TRANSPARENCY;

  *uvIntensity = adjustedIntensity;

  Serial.println("UV sensor glass transparency: " +
  String(UV_SENSOR_CAP_TRANSPARENCY));
  Serial.println("UV sensor level: " + String(uvLevel));
  Serial.println("Reference level: " + String(refLevel));
  Serial.println("Output voltage: " + String(outputVoltage));
  Serial.println("UV Intensity (mW/cm^2): " + String(mapped));
  Serial.println("Adjusted UV Intensity (mW/cm^2): " +
  String(adjustedIntensity));
  Serial.println("");
}

void getParticulateData(double*pm25, double*pm10) {
  Serial.println("SDS011");
  Serial.println("---");
  Serial.println("Waking up...");
  sds.wakeup();
  Serial.println("Inhaling for " + String(sdsInhalationSeconds) +
  " seconds...");

  delay(sdsInhalationSeconds * 1000);

  PmResult pm = sds.queryPm();
  if (pm.isOk()) {
    double newPm25 = pm.pm25;
    double newPm10 = pm.pm10;

    Serial.println("PM 2.5: " + String(newPm25));
    Serial.println("PM 10: " + String(newPm10));

    *pm25 = newPm25;
    *pm10 = newPm10;
  } else {
    Serial.print("Could not read values from sensor, reason: ");
    Serial.println(pm.statusToString());
  }

  WorkingStateResult state = sds.sleep();
  if (state.isWorking()) {
    Serial.println("Problem with sleeping the sensor.");
  } else {
    Serial.println("Asleep");
  }
  Serial.println("");
}

void getWindSpeedKmPerHr(double*windSpeed) {
  Serial.println("Anemometer");
  Serial.println("---");
  setActiveMultiplexerChannel(2);

  uint64_t startTime = millis();

  Serial.println("Start time: " + String(startTime));
  Serial.println("Measure for: " + String(windMeasurementSeconds) +
  " seconds...");

  int currentState = map(analogRead(ANALOGUE_IN), 0, 1024, 0, 1);
  int nextState = currentState;
  int stateChangeCount = 0;  // One state change corresponds to 1/4 rotation

  while ((uint64_t)(millis() - startTime) < windMeasurementSeconds * 1000) {
    nextState = map(analogRead(ANALOGUE_IN), 0, 1024, 0, 1);
    if (nextState != currentState) {
      stateChangeCount++;
      currentState = nextState;
    }
    delay(1);
  }

  Serial.println("End time: " + String(millis()));
  Serial.println("Recorded: " + String(stateChangeCount) +
  " state changes (1/4 rotations)");

  double windSpeedKmHr = static_cast<double>(stateChangeCount) /
  static_cast<double>(windMeasurementSeconds) * 1.2;

  Serial.println("Wind Speed: " + String(windSpeedKmHr) + " km/hr");
  Serial.println("");

  *windSpeed = windSpeedKmHr;
}

void sendData(double temperature, double humidity, double pressure,
  double uvIntensity, double rainfallLevel, double windSpeed, double pm25level,
  double pm10level) {
  dataJson["temperatureC"] = temperature;
  dataJson["airPressurePa"] = pressure;
  dataJson["humidityPercentage"] = humidity;
  dataJson["uvIntensityMilliwattsPerCmSq"] = uvIntensity;
  dataJson["rainfallStrengthPercentage"] = rainfallLevel;
  dataJson["windSpeedKmPerHour"] = windSpeed;
  dataJson["pm25density"] = pm25level;
  dataJson["pm10density"] = pm10level;

  Serial.println("Sending data");
  Serial.println("---");
  String dataJsonOutput;
  serializeJson(dataJson, dataJsonOutput);

  // todo: use the nicer http library

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
    Serial.println("Sent.");
  } else {
    Serial.println("ERROR: Failed to connect");
    reboot();
  }

  Serial.println("");
}

/**
 * The Arduino Map function but for floats
 * From: http://forum.arduino.cc/index.php?topic=3922.0
 */
double mapDouble(double x, double in_min, double in_max, double out_min,
  double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * Takes an average of readings on a given pin, returns the average
 */
int averageAnalogueRead(int pinToRead) {
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for (int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);
}

/**
 * System management functions
 */

void reboot() {
  delay(REBOOT_DELAY_MS);
  Serial.println("Rebooting...");
  ESP.restart();
}

void connectToWifi() {
  String wifiConnectionInfo = "Connecting to WiFi";

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.hostname(HOST_NAME);

  int connectAttempts = 0;
  int connectRetryInterval = 500;
  int rebootCountdown = WIFI_CONNECT_TIMEOUT_SECONDS * 1000;

  while (WiFi.status() != WL_CONNECTED) {
    delay(connectRetryInterval);

    Serial.print(".");

    rebootCountdown = rebootCountdown - connectRetryInterval;

    if (rebootCountdown < 0) {
      reboot();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
}
