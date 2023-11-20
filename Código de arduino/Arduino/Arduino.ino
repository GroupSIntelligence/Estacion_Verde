#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <DHT.h> // Agrega la biblioteca del sensor DHT11

// Configuración del sensor DHT11
#define DHTPIN 4    // El pin al que está conectado el sensor DHT11 (cambia esto según tu conexión)
#define DHTTYPE DHT11   // Tipo de sensor DHT (DHT11 o DHT22)

DHT dht(DHTPIN, DHTTYPE);

// Configuración de comunicación WiFi
SoftwareSerial wifi(2, 3);  // Configura los pines RX y TX para la comunicación con el módulo WiFi

// Configuración de depuración
#define DEBUG true

String sendDataToWifiBoard(String command, const int timeout, boolean debug);
String prepareDataForWifi(float temperatura, float humedad, float voltajet, float voltajep);
/**
 * Build and return a JSON document from the sensor data
*@param temperatura
*@param humedad
*@param voltajet Valor de voltaje de la turbina
*@param voltajet Valor de voltaje de los paneles
*@return
 */
String prepareDataForWifi(float temperatura, float humedad, float voltajet, float voltajep) {
  StaticJsonDocument<200> doc;

  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  doc["voltajet"] = voltajet;
  doc["voltajep"] = voltajep;

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  return jsonBuffer;
}

/**
 * Send data through Serial to ESP8266 module
 * @param command
 * @param timeout
 * @param debug
 * @return
 */
 
String sendDataToWifiBoard(String command, const int timeout, boolean debug) {
  String response = "";

  wifi.print(command);

  unsigned long time = millis();

  while ((time + timeout) > millis()) {
    while (wifi.available()) {
      char c = wifi.read();
      response += c;
    }
  }

  if (debug) {
    Serial.print(response);
  }

  return response;
}

void setup() {
  wifi.begin(9600);
  Serial.begin(9600);
  unsigned long stabilizingtime = 1000;
  dht.begin();
}

void loop() {
  if (DEBUG == true) {
    if (wifi.available()) {
        String espBuf;
        unsigned long time = millis();

        while ((time + 1000) > millis()) {
          while (wifi.available()) {
            char c = wifi.read();
            espBuf += c;
      }
    }
        Serial.print(espBuf);
    }
  }
    bool test = true;
    if (DEBUG == true) {

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();
  float voltajet = round(analogRead(0) * (5.0 / 1023.0) * 100) / 100.0;  // Redondea a dos decimales
  float voltajep = round(analogRead(1) * (5.0 / 1023.0) * 100) / 100.0;  // Redondea a dos decimales

          String preparedData = prepareDataForWifi(temperatura, humedad, voltajet, voltajep);
          sendDataToWifiBoard(preparedData, 4000, DEBUG);
          Serial.println(preparedData);
    }

  delay(500); // Espera 2 segundos antes de volver a leer el sensor
}


