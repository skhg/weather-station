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
#define WIFI_CONNECT_TIMEOUT_SECONDS 30

/**
 * Data logging server configuration
 */

#define SERVER_ADDRESS "http://192.168.178.29:8080"
#define URL_PATH "/weather-station/balcony"
#define HOSTNAME "weather-station"

/**
 * String defaults
 */
#define SEPARATOR_LINE "---"
#define HTTP_CONTENT_TYPE_HEADER "Content-Type"
#define HTTP_CONTENT_LENGTH_HEADER "Content-Length"
#define HTTP_JSON_CONTENT_TYPE "application/json"
#define EMPTY_STRING ""

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
#include <ESP8266HTTPClient.h>

/**
 * Instruments
 */
Adafruit_BME280 bme;
SdsDustSensor sds(SDS_DUST_RX, SDS_DUST_TX);

/**
 * Network messages
 */
WiFiClient WIFI_CLIENT;
HTTPClient HTTP_CLIENT;

/**
 * Runtime parameters
 */
uint64_t previousTime = 0;
float ultravioletDarknessVoltage = 1.07;
float ultravioletMaxVoltage = 2.8;
float maxUltravioletIntensityLevelAtLocation = 8.0;
int airMeasurementSeconds = 30;
int cycleIntervalSeconds = 80;  // Runs a bit more than every 2 minutes

void setup() {
  Serial.begin(115200);

  Serial.println("Booting");
  delay(REBOOT_DELAY_MS);  // Allow a moment for some sensors to initialise

  previousTime = millis();

  setupMultiplexer();
  setupInstruments();

  Serial.println("Setup complete!");
  Serial.println(EMPTY_STRING);
}

void loop() {
  double temperature;
  double humidity;
  double pressure;

  getBme280data(&temperature, &pressure, &humidity);

  double uvIntensity;
  getUvSensorData(&uvIntensity);

  startParticulateMeasurement();

  double windSpeed;
  getWindSpeedKmPerHr(&windSpeed);

  double pm25level;
  double pm10level;
  endParticulateMeasurement(&pm25level, &pm10level);

  if (connectToWifi()) {
    sendData(temperature, humidity, pressure, uvIntensity,
      windSpeed, pm25level, pm10level);
  }

  WiFi.disconnect();
  WiFi.forceSleepBegin();

  sleepUntilNext();
}

void sleepUntilNext() {
  Serial.println("Cycle complete.");
  Serial.println(SEPARATOR_LINE);
  Serial.println("Sleeping for " + String(cycleIntervalSeconds) +
  " seconds...");
  Serial.println(EMPTY_STRING);
  Serial.flush();

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
  Serial.println(EMPTY_STRING);

  Serial.println("Setup SDS011...");
  // Dust sensor
  sds.begin();
  Serial.println(sds.queryFirmwareVersion().toString());
  Serial.println(sds.setQueryReportingMode().toString());
  sds.sleep();
  Serial.println(EMPTY_STRING);
}

void getBme280data(double*temperature, double*pressure, double*humidity) {
  double liveTemp = bme.readTemperature();
  double livePressure = bme.readPressure();
  double liveHumidity = bme.readHumidity();

  Serial.println("BME280");
  Serial.println(SEPARATOR_LINE);
  Serial.println("Temperature: " + String(liveTemp));
  Serial.println("Pressure: " + String(livePressure));
  Serial.println("Humidity: " + String(liveHumidity));
  Serial.println(EMPTY_STRING);

  *temperature = liveTemp;
  *pressure = livePressure;
  *humidity = liveHumidity;
}

void getUvSensorData(double*uvIntensity) {
  Serial.println("ML8511");
  Serial.println(SEPARATOR_LINE);
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
  Serial.println(EMPTY_STRING);
}

void startParticulateMeasurement() {
  Serial.println("SDS011");
  Serial.println(SEPARATOR_LINE);
  Serial.println("Waking up...");
  sds.wakeup();
}

void endParticulateMeasurement(double*pm25, double*pm10) {
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
  Serial.println(EMPTY_STRING);
}

void getWindSpeedKmPerHr(double*windSpeed) {
  Serial.println("Anemometer");
  Serial.println(SEPARATOR_LINE);
  setActiveMultiplexerChannel(2);

  uint64_t startTime = millis();

  Serial.print("Start time: ");
  Serial.println(millis());
  Serial.println("Measure for: " + String(airMeasurementSeconds) +
  " seconds...");

  int currentState = map(analogRead(ANALOGUE_IN), 0, 1024, 0, 1);
  int nextState = currentState;
  int stateChangeCount = 0;  // One state change corresponds to 1/4 rotation

  while ((uint64_t)(millis() - startTime) < airMeasurementSeconds * 1000) {
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
  static_cast<double>(airMeasurementSeconds) * 1.2;

  Serial.println("Wind Speed: " + String(windSpeedKmHr) + " km/hr");
  Serial.println(EMPTY_STRING);

  *windSpeed = windSpeedKmHr;
}

void sendData(double temperature, double humidity, double pressure,
  double uvIntensity, double windSpeed, double pm25level,
  double pm10level) {
  StaticJsonDocument<JSON_OBJECT_SIZE(8) + 1000> dataJson;

  dataJson["temperatureC"] = temperature;
  dataJson["airPressurePa"] = pressure;
  dataJson["humidityPercentage"] = humidity;
  dataJson["uvIntensityMilliwattsPerCmSq"] = uvIntensity;
  dataJson["windSpeedKmPerHour"] = windSpeed;
  dataJson["pm25density"] = pm25level;
  dataJson["pm10density"] = pm10level;

  Serial.println("Sending data");
  Serial.println(SEPARATOR_LINE);
  String jsonString;
  serializeJson(dataJson, jsonString);

  HTTP_CLIENT.begin(WIFI_CLIENT, String(SERVER_ADDRESS) + String(URL_PATH));

  HTTP_CLIENT.addHeader(HTTP_CONTENT_TYPE_HEADER, HTTP_JSON_CONTENT_TYPE);
  HTTP_CLIENT.addHeader(HTTP_CONTENT_LENGTH_HEADER,
    String(jsonString.length()));
  int result = HTTP_CLIENT.POST(jsonString);

  Serial.print("Completed with response code: ");
  Serial.println(result);

  Serial.flush();
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

boolean connectToWifi() {
  Serial.println(SEPARATOR_LINE);

  WiFi.forceSleepWake();

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int connectAttempts = 0;
  int connectRetryInterval = 500;
  int rebootCountdown = WIFI_CONNECT_TIMEOUT_SECONDS * 1000;

  while (WiFi.status() != WL_CONNECTED) {
    delay(connectRetryInterval);

    Serial.print(".");

    rebootCountdown = rebootCountdown - connectRetryInterval;

    if (rebootCountdown < 0) {
      Serial.println(EMPTY_STRING);
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(EMPTY_STRING);
  Serial.println("WiFi connected");
  return true;
}
