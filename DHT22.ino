#include <DHT.h>
#include <DHT_U.h>

#include <Adafruit_WINC1500.h>
#include <Adafruit_WINC1500Server.h>
#include <Adafruit_WINC1500Udp.h>
#include <Adafruit_WiFiMDNSResponder.h>
#include <Adafruit_WINC1500Client.h>
#include <Adafruit_WINC1500SSLClient.h>

#include "ArduinoPrivate.h"

#define DHTPIN 2     // what digital pin we're connected to

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC

#define webpage "/dhtsensor/"         // path to test page

#define nextReading 2700000  //45 Minutes
//#define nextReading 1800000  //30 Minute
//#define nextReading 300000  //5 Minute
//#define nextReading 60000   //1 Minute


// Setup the WINC1500 connection with the pins above and the default hardware SPI.
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);

char basicAuthorization[] = API_AUTH;
char ssid[] = NET_SSID;
char pass[] = NET_SSID_PASS;
const unsigned int port = SERVER_PORT;
int keyIndex = 0;                             //network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
char server[] = SERVER;                  // domain name for test page (using DNS)

// Initialize the Ethernet client library with the IP address and port of the server
Adafruit_WINC1500Client client;

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds

int sensorPin = 0;
String tempData;
unsigned long time;

void setup()
{
#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  Serial.println("DHT Temperature Sensor Data Collection");

  dht.begin();

  // Wait a few seconds.
  delay(2000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop()                     // run over and over again
{
  getTempData();

}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// this method makes a HTTP connection to the server:
void httpRequest(String tempData) {

  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

Serial.print("IoT Server: ");
Serial.println(server);

//Serial.print("Temperature Data: ");
//Serial.println(tempData);

  // if there's a successful connection:
  if (client.connect(server, port)) {
    Serial.println();
    Serial.println();
    Serial.println("Establish Connection to IoT Server ");
    Serial.println(server);
    Serial.println("Connected...");
    Serial.println();
    Serial.println(); 

 
    // Make a HTTP request: 
    
    char postMessage[100];
    sprintf(postMessage," POST %s HTTP/1.1", webpage);

    client.println(postMessage);

    client.print("Content-Type: ");
    client.println("application/json");

    client.print("Content-Length: ");
    client.println(tempData.length());

    client.print("Authorization: ");
    client.println(basicAuthorization);

    client.print("Host: "); 
    client.println(server); 

    client.println("Connection: close");

    client.println();
    client.println(tempData);

//Debug
    Serial.println(tempData);

  }
  else {
    // if you couldn't make a connection:
    Serial.println("Connection to IoT Server Failed");
    Serial.println(tempData);
  }

}


void getTempData() {

 // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  tempData.concat("{");
  tempData.concat("\"humidity\": ");
  tempData.concat("\"");
  tempData.concat(h);
  tempData.concat("\"");
  tempData.concat(", ");
  tempData.concat("\"celsius\": ");
  tempData.concat("\"");
  tempData.concat(t);
  tempData.concat("\"");
  tempData.concat(", ");
  tempData.concat("\"fahrenheit\": ");
  tempData.concat("\"");
  tempData.concat(f);
  tempData.concat("\"");
  tempData.concat(", ");
  tempData.concat("\"heatindex\": ");
  tempData.concat("\"");
  tempData.concat(hif); 
  tempData.concat("\"");
  tempData.concat("} ");

  httpRequest(tempData);

  tempData = "";

  // Take a temperature reading every 10 Minutes
  delay(nextReading);
}
