#include <time.h>
#include "secrets.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h> // Librería para trabajar con JSON
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <WiFiClientSecure.h>

#define DEBUG true // Habilita o deshabilita la depuración

const int MQTT_PORT         = 8883;
const char MQTT_PUB_TOPIC[] = "esp8266/pub"; // Tema MQTT para publicación

uint8_t DST = 0; // Horario de verano (Daylight Saving Time) deshabilitado
WiFiClientSecure net; // Cliente WiFi seguro

// Configuración de comunicación Serial entre el ESP8266 y Arduino Uno
SoftwareSerial UnoBoard(10, 11);

// Certificados y claves para la comunicación segura
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

PubSubClient client(net); // Cliente MQTT

unsigned long lastMillis = 0;
time_t now;
time_t nowish = 1510592825; // Tiempo de referencia para la configuración de hora

// Declaración de funciones
void connectToMqtt();
void NTPConnect(void);
void sendDataToAWS(void);
void checkWiFiThenMQTT(void);
void connectToWiFi(String init_str);
void messageReceived(char *topic, byte *payload, unsigned int length);
String sendDataToUno(String command, const int timeout, boolean debug);

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  sendDataToUno("Setting time using SNTP\r\n", 1000, DEBUG);

  // Configurar la hora usando SNTP (Simple Network Time Protocol)
  configTime(TIME_ZONE * 3600, DST * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);

  while (now < nowish) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println(" done!");
  sendDataToUno(" done!\r\n", 1000, DEBUG);

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

void connectToMqtt()
{
  Serial.print("MQTT connecting ");
  sendDataToUno("MQTT connecting \r\n", 1000, DEBUG);

  while (!client.connected()) {
    // Intentar conectarse al servidor MQTT
    if (client.connect(THINGNAME)) {
      Serial.println("connected!");
      sendDataToUno("connected! \r\n", 1000, DEBUG);
    } else {
      Serial.print("failed, reason -> ");
      Serial.println(client.state());
      Serial.println(" < try again in 5 seconds");
      delay(5000);
    }
  }
}

void connectToWiFi(String init_str)
{
  Serial.print(init_str);

  // Configurar nombre de host y conectarse a la red WiFi
  WiFi.hostname(THINGNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" ok!");
}

void checkWiFiThenMQTT(void)
{
  connectToWiFi("Checking WiFi");
  sendDataToUno("Checking WiFi \r\n", 1000, DEBUG);

  connectToMqtt();
}

void sendDataToAWS(void)
{
  StaticJsonDocument<200> doc;

  // Leer datos provenientes de la placa Uno y almacenarlos en la variable "doc"
  DeserializationError error = deserializeJson(doc, Serial.readString());

  // Verificar si el proceso de lectura fue exitoso.
  if (error) {
    Serial.print("deserializeJson() failed.");
    return;
  }

  // Si el proceso de lectura fue exitoso, continuar y establecer la hora.
  doc["time"] = String(millis());

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
  if (!client.publish(MQTT_PUB_TOPIC, jsonBuffer, false)) {
    Serial.println(client.state());
  }
}

String sendDataToUno(String command, const int timeout, boolean debug)
{
  String response = "";
  UnoBoard.print(command); // Enviar el carácter de lectura al Uno
  long int time = millis();

  while( (time+timeout) > millis()) {
    while(UnoBoard.available()) {
      // El ESP tiene datos, así que muestra su salida en la ventana serial
      char c = UnoBoard.read(); // Leer el siguiente carácter.
      response+=c;
    }
  }

  if (debug) {
    Serial.print(response);
  }

  return response;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("starting setup");

  UnoBoard.begin(9600); // Velocidad de baudios del ESP8266 (puede ser diferente)
  delay(2000);

  connectToWiFi(String("Attempting to connect to SSID: ") + String(ssid));

  NTPConnect();

  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(messageReceived);

  connectToMqtt();
  Serial.println("end setup");
}

void loop()
{ 
  now = time(nullptr);
  if (!client.connected()) {
    checkWiFiThenMQTT();
  } else {
    client.loop();
    if (millis() - lastMillis > 5000) {
      lastMillis = millis();
      sendDataToAWS();
    }
  }
}
