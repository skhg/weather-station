/* Flame Sensor analog example.
Code by Reichenstein7 (thejamerson.com)

For use with a Rain Sensor with an analog out!

To test view the output, point a serial monitor such as Putty at your Arduino. 

  - If the Sensor Board is completely soaked; "case 0" will be activated and " Flood " will be sent to the serial monitor.
  - If the Sensor Board has water droplets on it; "case 1" will be activated and " Rain Warning " will be sent to the serial monitor.
  - If the Sensor Board is dry; "case 2" will be activated and " Not Raining " will be sent to the serial monitor. 

*/

#define RAINFALL_IN A0

#define MULTIPLEX_S0 14 // D5
#define MULTIPLEX_S1 12 // D6
#define MULTIPLEX_S2 13 // D7
#define MULTIPLEX_S3 15 // D8

// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

void setup() {
  // initialize serial communication @ 9600 baud:
  Serial.begin(9600);  

  pinMode(MULTIPLEX_S0, OUTPUT);
  pinMode(MULTIPLEX_S1, OUTPUT);
  pinMode(MULTIPLEX_S2, OUTPUT);
  pinMode(MULTIPLEX_S3, OUTPUT);

  //make input 1 active
  digitalWrite(MULTIPLEX_S0, HIGH);
  digitalWrite(MULTIPLEX_S1, LOW);
  digitalWrite(MULTIPLEX_S2, LOW);
  digitalWrite(MULTIPLEX_S3, LOW);
  
}
void loop() {
  // read the sensor:
  int sensorReading = analogRead(RAINFALL_IN);
  // map the sensor range (four options):
  // ex: 'long int map(long int, long int, long int, long int, long int)'
  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);



  Serial.println(sensorReading);
  
  // range value:
//  switch (range) {
// case 0:    // Sensor getting wet
//    Serial.println("Flood");
//    break;
// case 1:    // Sensor getting wet
//    Serial.println("Rain Warning");
//    break;
// case 2:    // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
//    Serial.println("Not Raining");
//    break;
//  }




  
  delay(100);  // delay between reads
}
