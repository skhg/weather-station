//More information at: http://www.aeq-web.com/?ref=arduinoide

const int m_time = 5;      //Meassuretime in Seconds
int wind_ct = 0;
float wind = 0.0;
unsigned long current_time = 0;

void ICACHE_RAM_ATTR countWind();


void setup()
{
  Serial.begin(9600);
  current_time = millis();
}

void loop()
{

  meassure();

  Serial.print("Windgeschwindigkeit: ");
  Serial.print(wind);       //Speed in Km/h
  Serial.print(" km/h - ");
  Serial.print(wind / 3.6); //Speed in m/s
  Serial.println(" m/s");

}

void countWind() {
  wind_ct ++;
}

void meassure() {
  wind_ct = 0;
  current_time = millis();
  attachInterrupt(D1, countWind, RISING);
  delay(1000 * m_time);
  detachInterrupt(D1);
  wind = (float)wind_ct / (float)m_time * 1.2; // 2 pulses per rotation
}
