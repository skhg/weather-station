#define ANEMOMETER_IN A0

#define MULTIPLEX_S0 14 // D5
#define MULTIPLEX_S1 12 // D6
#define MULTIPLEX_S2 13 // D7
#define MULTIPLEX_S3 15 // D8

const int measurementIntervalSeconds = 5;
int wind_ct = 0;
unsigned long previousTime = 0;
int windState = -1;

void setup()
{
  Serial.begin(9600);

  pinMode(MULTIPLEX_S0, OUTPUT);
  pinMode(MULTIPLEX_S1, OUTPUT);
  pinMode(MULTIPLEX_S2, OUTPUT);
  pinMode(MULTIPLEX_S3, OUTPUT);

  //make input 2 active
  digitalWrite(MULTIPLEX_S0, LOW);
  digitalWrite(MULTIPLEX_S1, HIGH);
  digitalWrite(MULTIPLEX_S2, LOW);
  digitalWrite(MULTIPLEX_S3, LOW);
  
  previousTime = millis();
}

void loop()
{
  unsigned long currentTime = millis();

  int anemometerState = map(analogRead(ANEMOMETER_IN), 0, 1024, 0, 1);

  if ((unsigned long)(currentTime - previousTime) >= measurementIntervalSeconds * 1000 ) {
      float wind = measure();

      Serial.print("Windgeschwindigkeit: ");
      Serial.print(wind);       //Speed in Km/h
      Serial.print(" km/h - ");
      Serial.print(wind / 3.6); //Speed in m/s
      Serial.println(" m/s");
        
      previousTime = currentTime;
   }
   
  if(windState == -1){
    // Just booted so set it
    windState = anemometerState;
  } else {
    
    //todo debounce based on change time < 10 millis, maybe not needed
    
    if(windState != anemometerState){
      wind_ct ++; //corresponds to 1/4 rotation
      windState = anemometerState;
    }
  }
}

float measure() {
  float wind = (float)wind_ct / (float)measurementIntervalSeconds * 1.2; // 2 on/off cycles per rotation (1 spin = 2.4 km/hr apparently)
  wind_ct = 0;
  return wind;
}
