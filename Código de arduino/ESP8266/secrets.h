#include <avr/pgmspace.h>

#define SECRET 
 
const char ssid[] = " "; // Reemplaza con tu SSID
const char pass[] = " "; // Reemplaza con tu contraseña WiFi

#define THINGNAME " "

int8_t TIME_ZONE = -5;

const char MQTT_HOST[] = " ";

static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----


-----END CERTIFICATE-----
)EOF";
static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----


-----END CERTIFICATE-----
)KEY";

static const char privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";