# weather-station
I wanted to experiment with building environmental sensors, and see if I could make homemade electronics that would survive in the outdoors. So I built a weather station based around the Arduino-compatible ESP8266 platform.

# Overview
There are many possible instruments that can be part of a weather station. For the amateur meteorologist, just a few are needed to make a start. They can all be bought cheaply online, and a weatherproof case can be made out of just a few short lengths of PVC piping. I based the system on this [reference design](https://sensor.community/en/sensors/airrohr/), with some additions for the extra sensors I wanted. The software is custom-made.

With this system we can measure:
 * Wind speed
 * Temperature
 * Relative humidity
 * Air pressure
 * Air particulate matter (PM2.5 and PM10)
 * Solar U.V. intensity
 * Rainfall (sort-of)
 
This is a wall-mounted device so wind direction wasn't really something I cared about for this build.

## Materials required

Sensors and electronics:

* [BME280](https://www.amazon.de/-/en/gp/product/B07D8T4HP6/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) - Temperature, Humidity, and Air Pressure sensor
* [SDS011](https://www.amazon.de/-/en/gp/product/B07911ZY9W/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) - Air particulate matter sensor (PM2.5 and PM10)
* [MH-RD](https://www.amazon.de/-/en/AZDelivery-Rain-Sensor-Module-Parent/dp/B07V5QQW9J/ref=sr_1_9?dchild=1&keywords=arduino%2Bregensensor&qid=1588014518&sr=8-9&th=1) - Rainfall droplet sensor
* [Froggit](https://www.amazon.de/-/en/gp/product/B00GGM5HEA/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) - Anemometer
* [GY-8511](https://www.amazon.de/-/en/gp/product/B07PQPHJKR/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1) - Ultraviolet intensity sensor
* [NodeMCU ESP8266](https://www.amazon.de/-/en/gp/product/B074Q2WM1Y/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) - Arduino-compatible microcontroller
* [JST headers](https://www.amazon.de/YIXISI-Connector-JST-XH-Female-Adapter/dp/B082ZLYRRN/ref=sr_1_1_sspa?dchild=1&keywords=jst+kit&qid=1605108398&sr=8-1-spons&psc=1&spLa=ZW5jcnlwdGVkUXVhbGlmaWVyPUFITU9OODc2TFg4MzEmZW5jcnlwdGVkSWQ9QTAxNTU4NzMyOVdJRFVETUhCV1Y3JmVuY3J5cHRlZEFkSWQ9QTAwODA5ODVQWFZCU00yNTJBSlYmd2lkZ2V0TmFtZT1zcF9hdGYmYWN0aW9uPWNsaWNrUmVkaXJlY3QmZG9Ob3RMb2dDbGljaz10cnVl)
* Digital/Analogue [multiplexer](https://www.amazon.de/-/en/gp/product/B06Y1L95GK/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* 10kΩ [resistor](https://www.amazon.de/-/en/gp/product/B07Q87JZ9G/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* [Ribbon cable](https://www.amazon.de/-/en/gp/product/B076CLY8NH/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* [Perfboard](https://www.amazon.de/-/en/gp/product/B07BDKG68Q/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* USB power supply

Case construction:

* 2x [45-degree drainpipe elbow](https://www.obi.de/ht-boegen/marley-ht-bogen-45-dn-75-grau/p/7436181)
* [Drainpipe with "cleaning" cap](https://www.obi.de/ht-reinigungsrohre/marley-ht-reinigungsrohr-dn-75/p/7434681)
* 2x [Gutter bracket](https://www.obi.de/kunststoff-dachrinnen/marley-rinnenhalter-verstellbar-dn-75-grau/p/5088869)
* Wooden board for mounting as required
* Insulating plastic/foam liner

UV Sensor module:
* Some little [wooden boards](https://www.amazon.de/-/en/gp/product/B07D76MKFY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* Small glass dome / [clock glass](https://www.amazon.de/-/en/gp/product/B00FWSFHBW/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1)
* Weather-resistant cabling
* Waterproof rubber gasket

## Assembly

## Circuit

## Software

## Data Capture and Visualisation

# References
* https://hackaday.io/project/165061-solar-powered-wifi-weather-station-v20#j-discussions-title
* https://www.geeky-gadgets.com/arduino-wind-speed-meter-anemometer-project-30032016/
* https://learn.sparkfun.com/tutorials/ml8511-uv-sensor-hookup-guide/all
* https://gist.github.com/geoffwatts/b0b488b5a5257223ed53
* https://sensor.community/en/sensors/
* https://sensor.community/en/sensors/airrohr/
* https://learn.pimoroni.com/tutorial/sandyj/enviro-plus-and-luftdaten-air-quality-station
* https://www.aeq-web.com/anemometer-mit-dem-arduino-bauen/
* Handling interrupts https://forum.arduino.cc/index.php?topic=616264.0
