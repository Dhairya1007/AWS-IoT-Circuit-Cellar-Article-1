

/*
 * Circult Cellar AWS IoT Article Series ( Part 1 ) 
 * Author - Dhairya Parikh
 * Description - This code is written for the NodeMCU Development board. The primary purpose of this 
 * code to connect to AWS IoT Core and print the DHT11 sensor data on the Node1/DHT11 topic. Moreover,
 * it will subscribe to Node1/subscribe topic and print any messages that arrive on this topic.
 */

// ------------------ Importing the required Libraries -------------------------

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "DHT.h"
#include "secrets.h"      // We will create this file to store all our sensitive data. Just go to the down arrow button on the top right and press on "Create New Tab"

// ----------------------------------------------------------------------------

// --------------------- Variable, Object and constant Declarations -------------

// BMP 280 Sensor value variables
float temperature ;
float humidity ;

// Variables to implement publishing of sensor data to AWS every 5 seconds
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;
const int DHTPin = 14;
const int onChipLED = 2;

// variables to store the latest time fetched by the NTP Client.
time_t now;
time_t nowish = 1510592825;

// AWS Publish and subscribe topic mentioned in the AWS policy you created 
#define AWS_PUBLISH_TOPIC   "Node1/DHT11"
#define AWS_SUBSCRIBE_TOPIC "Node1/subscribe"

// WiFi SSL to add the certificates and the private pem key we retrived from AWS 
WiFiClientSecure net;
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

// Objects for pubsub and BMP sensor
PubSubClient client(net);
DHT dht;

// --------------------------------------------------------------------------

// NTPConnect : This function fetches the latest time. 
// This is required for connecting with AWS IoT (Used for authentication)
 
void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// ---------------------------------------------------------------------------------------------

// Custom function for Wifi connection establishment

void Setup_WiFi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println(String("Attempting to connect to the following WiFi Network: ") + String(WIFI_SSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected!");
}

// ---------------------------------------------------------------------------------------------

// Callback Funtion to print any incoming messages on the subscribed topics

void callbackFunc(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;

  Serial.print(". Message: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    message += (char)message[i];
  }
  Serial.println();

   // Feel free to add more if statements to control more GPIOs using MQTT Topics
  // If a message is received on the topic Node1/subscribe, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="Node1/subscribe"){ 
      if(message == "on"){
        Serial.print("Turning on on-chip LED");
        digitalWrite(onChipLED, LOW);
      }
      else if(message == "off"){
        Serial.print("Turning on on-chip LED");
        digitalWrite(onChipLED, HIGH);
      }
  }
  Serial.println();
}

// ---------------------------------------------------------------------------------------------

// Connect to AWS IoT Thing entity we created.

void connectAWS()
{
  delay(3000);

  Setup_WiFi();
  NTPConnect();
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(callbackFunc);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(1000);
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

// ---------------------------------------------------------------------------------------------

// Publish DHT11 data along with latest timestamp on the Node1/DHT11 topic
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_PUBLISH_TOPIC, jsonBuffer);
}

// ---------------------------------------------------------------------------------------------

// Setup Function
 
void setup()
{
  Serial.begin(115200);
  pinMode(onChipLED, OUTPUT);
  
  // Initialize the DHT11 Sensor
  dht.setup(DHTPin);

  // Calling the connectAWS() function to establish a connection with our AWS IoT thing
  connectAWS();
}
 
// ---------------------------------------------------------------------------------------------

// Loop Function 

void loop()
{
  // Reading Humidity and Temperature values
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  
  delay(2000); 
  now = time(nullptr);
  
  if (!client.connected())
  {
    connectAWS();
  }
  else
  {
    client.loop();
    if (millis() - lastMillis > 5000)
    {
      lastMillis = millis();
      publishMessage();
    }
  }
}

// ---------------------------------------------------------------------------------------------
