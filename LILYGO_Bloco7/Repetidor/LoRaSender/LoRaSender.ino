#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include "boards.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define QTD_SENSORES 10

// Replace with your network credentials
const char* ssid     = "carlos vieira";
const char* password = "jonathan";
const char* mqttServer = "mqtt.tago.io";
const int mqttPort = 1883;
const char* mqttUser = "jonathan";
const char* mqttPassword = "a28a9f08-0be5-4dcc-a2ce-91c9f1089b83";

char dado[9];
char temperatura[90];
WiFiClient espClient;
PubSubClient client(espClient);
float pacote[QTD_SENSORES + 2];

void setup()
{
    Serial.begin(115000);
    initBoard();
    delay(1500);

    Serial.println("Iniciando Comunicação LoRa");
    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Falha ao tentar iniciar a comunicação LoRa");
        while (1);
    }

  Serial.print("Iniciando a Comunicação WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Conectando à rede Wi-Fi");
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) 
  {
    Serial.println("Conectando ao MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) 
    {
      Serial.println("Conectado");
    } 
    else 
    {
      Serial.print("Falha ao iniciar!");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop()
{
  int packet_size = LoRa.parsePacket();

  if (packet_size > 0) 
  {
    // Lê os bytes recebidos e os armazena em pacote
    int read = LoRa.readBytes((uint8_t *)pacote, sizeof(pacote));

    if(pacote[0] == 2.0)
    {
      for(int i = 0; i < QTD_SENSORES + 2 ; i++)
      {
        client.loop();
        sprintf(dado,"t%d = %.2f", i, pacote[i]);
        client.publish("topic",dado);
        Serial.println(dado);
        delay(3000);
      }
    }
  }
}


