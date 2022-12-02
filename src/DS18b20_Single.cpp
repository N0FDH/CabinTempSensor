
/*   ---------
    | DS18B20 |
    |Flat Side|
     ---------
     |   |   |
   GND  Data VCC 3.3V


     ---------
    | ESP8266 |
    | Top     |
    | Side    |
    |         |
    | 1 2 3 4 |
    | 0 0 0 0 |
    |         |
    | 0 0 0 0 |
    | 5 6 7 8 |
     ---------

     Pin #1 - GND
     Pin #2 - GPIO 2  Connect Data pin to DS18B20
     Pin #3 - GPIO 0
     Pin #4 - RX
     Pin #5 - TX
     Pin #6 - CH_PD
     Pin #7 - Reset
     Pin #8 - VCC 3.3



     A ------| 4.7K Resistor |-------- B

     A - connect to ESP8266 GPIO 2
     B - connect to DS18B20 Data Pin

*/

// Including library's
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "WiFiCredentials.h"

#define ONE_WIRE_BUS 2 // Data wire is plugged into the ESP8266-01 GPIO - 2

#if 0 // Loren
// Update these with values suitable for your network.
IPAddress ip(192, 168, 1, 128);    // Loren's Proto-Type ESP8266 static IP
IPAddress gateway(192, 168, 1, 1); // Loren's gateway
IPAddress subnet(255, 255, 255, 0);

#else // Randy
// Update these with values suitable for your network.
// IPAddress ip(192,168,0,191);  //Temp Sensor #1 ESP8266 static IP
// IPAddress ip(192,168,0,192);  //Temp Sensor #2 ESP8266 static IP
// IPAddress ip(192,168,0,193);  //Temp Sensor #3 ESP8266 static IP
IPAddress ip(192, 168, 0, 194); // Temp Sensor #4 ESP8266 static IP

IPAddress gateway(192, 168, 0, 1); // Randy's gateway
IPAddress subnet(255, 255, 255, 0);

#endif

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);
char temperatureFString[16]; // Fahrenheit

// Web Server on port 80
WiFiServer server(80);

// Setup will only run once on boot
void setup()
{
  // Initializing serial port for debugging purposes
  // Serial.begin(115200);
  // delay(10);

  DS18B20.begin(); // IC Default 9 bit. If you have troubles could try upping it 12.
                   // Ups the delay giving the IC more time to process the temperature measurement

  WiFi.enableAP(false); // Workaround to be sure AP mode is off
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  // Starting the web server
  server.begin();

  delay(5000);
}

void getTemperature()
{
  float tempF, tempC;
  do
  {
    DS18B20.requestTemperatures();
    tempF = DS18B20.getTempFByIndex(0);
    tempC = DS18B20.getTempCByIndex(0);
    dtostrf(tempF, 3, 2, temperatureFString);
    delay(100);
  } while (tempC == 85.0 || tempC == (-127.0));
}

// runs over and over again
void loop()
{
  // Listenning for new clients
  WiFiClient client = server.available();

  if (client)
  {
    // Serial.println("New client");
    //  bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();

        if (c == '\n' && blank_line)
        {
          getTemperature();
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          client.println("<!DOCTYPE HTML>"); // your actual web page that displays temperature
          client.println("<html>");
          client.println(temperatureFString);
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r')
        {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }
    // closing the client connection
    delay(1);
    client.stop();
  }
}

