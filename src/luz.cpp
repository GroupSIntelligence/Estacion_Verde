#include <Arduino.h>
#include <ArduinoJson.h>
//#include <ESP8266WiFi.h>  // Esta línea está comentada, por lo que no se utiliza
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

// Declaración de funciones
String sendDataToWifiBoard(String command, const int timeout, boolean debug);
String prepareDataForWifi(float temperatura, float humedad, float voltajet, float voltajel);
float readLuminosity();  // Función para leer la luminosidad
/**
 * Construye y devuelve un documento JSON a partir de los datos del sensor
 * @param temperatura Valor de temperatura
 * @param humedad Valor de humedad
 * @param voltajet Valor de voltaje de la turbina
 * @param voltajel Valor de voltaje de los paneles solares
 * @return Cadena JSON
 */
String prepareDataForWifi(float temperatura, float humedad, float voltajet, float voltajel) {
  StaticJsonDocument<200> doc;

  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  doc["voltajet"] = voltajet;
  doc["voltajel"] = voltajel;

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  return jsonBuffer;
}

/**
 * Envía datos a través del puerto Serial al módulo ESP8266
 * @param command Comando a enviar
 * @param timeout Tiempo de espera
 * @param debug Habilita la depuración
 * @return Respuesta del módulo
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
  wifi.begin(9600);  // Inicializa la comunicación con el módulo WiFi
  Serial.begin(9600);  // Inicializa la comunicación serial
  unsigned long stabilizingtime = 2000;  // Tiempo de estabilización (no se utiliza aquí)
  dht.begin();  // Inicializa el sensor DHT11
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

  float temperatura = dht.readTemperature();  // Lee la temperatura del sensor DHT11
  float humedad = dht.readHumidity();  // Lee la humedad del sensor DHT11
  float voltajet = readVoltajet();
  float voltajel = readVoltajel();


  while(test == true){
    if (DEBUG == true) {
          String preparedData = prepareDataForWifi(temperatura, humedad, voltajet, voltajel);
          // preparedData += " Voltaje: " + String(voltaje) + " %";  // Agrega la luminosidad a los datos
          sendDataToWifiBoard(preparedData, 4000, DEBUG);
          Serial.println(preparedData);
    }
  }
  
  delay(2000); // Espera 2 segundos antes de volver a leer el sensor
}

// Función para leer la luminosidad (código optimizado)
float readVoltajet() {
  return 0.0049 * analogRead(A0);  // Lee la luminosidad y realiza el reescalado
 //* (3.3 / 1023.0); //escalamos a voltaje 
}
float readVoltajel() {
  return 0.0049 * analogRead(A0);  // Lee la luminosidad y realiza el reescalado
 //* (3.3 / 1023.0); //escalamos a voltaje cambiar analogRead
}
