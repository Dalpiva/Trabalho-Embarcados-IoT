  //WIFI
// Load Wi-Fi library
#include <WiFi.h>
#include <PubSubClient.h>

// Replace with your network credentials
const char* ssid     = "carlos vieira";
const char* password = "jonathan";
const char* mqttServer = "mqtt.tago.io";
const int mqttPort = 1883;
const char* mqttUser = "jonathan";
const char* mqttPassword = "9e9564e5-13f6-425f-b756-6a0b8fc785b4";

int temperatura = 25;

float pacote[12];
float normal; 

char dado[9];

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115000);

  //CONFIGURAÇÃO MQTT
   // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) 
    {
      Serial.println("connected");
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  sprintf(dado,"t = %.2f",temperatura);
  client.publish("topic",dado);
  Serial.println(dado);
}

void loop() {
  if(Serial.available()) {
    dado = Serial.parseFloat(); 
    Serial.println(normal);
  }
  
  client.loop();
  sprintf(dado,"t = %d",temperatura);
  client.publish("topic",dado);
  Serial.println(dado);

  delay(5000);
}


