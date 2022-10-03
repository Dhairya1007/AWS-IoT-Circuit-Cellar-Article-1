#include <pgmspace.h>
 
#define SECRET
 
const char WIFI_SSID[] = "WIFI SSID";               //Wifi SSID Name
const char WIFI_PASSWORD[] = "WIFI PASSWORD";           //Wifi Password
 
#define THINGNAME "Thing_Name"    // Name of your thing in AWS
 
int8_t TIME_ZONE = +5.5; //IST(India): +5.5 UTC
 
const char MQTT_HOST[] = "xxxxxxxxx.iot.<AWS Zone>.amazonaws.com"; 
// Get this from the AWS IoT Core Settings
 
 
static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
..............
-----END CERTIFICATE-----
)EOF";
 
 
// Copy contents from XXXXXXXX-certificate.pem.crt here ▼
static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
.............
-----END CERTIFICATE-----

)KEY";
 
 
// Copy contents from  XXXXXXXX-private.pem.key here ▼
static const char privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
.............
-----END RSA PRIVATE KEY-----

)KEY";
