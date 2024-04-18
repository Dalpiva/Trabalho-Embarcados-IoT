#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>

#include "boards.h"
 
/* Definicoes para comunicação com radio LoRa */
/*      Pinos LoRa          Pinos ESP32        */

 
#define HIGH_GAIN_LORA      20  /* dBm */

/* Definicoes gerais */
#define DEBUG_SERIAL_BAUDRATE    115200

#define QTD_SENSORES        10 + 2
 
/* Local prototypes */
void display_init(void);
bool init_comunicacao_lora(void);
bool init_comunicacao_lora2(void);

float pacote[QTD_SENSORES];

/* Funcao de setup */
void setup() 
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    Serial.println("LoRa Sender");
    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }  
}
 
/* Programa principal */
void loop() 
{
  int packet_size = LoRa.parsePacket();
  if (packet_size > 0) 
  {
    // Lê os bytes recebidos e os armazena em pacote
    int read = LoRa.readBytes((uint8_t *)pacote, sizeof(pacote));

    if(pacote[0] == 1.00)
    {
      Serial.println(pacote[0]);
      pacote[0] = 2.0;

      LoRa.beginPacket();
      LoRa.write((uint8_t *)pacote, sizeof(pacote));
      LoRa.endPacket();
    }
  }
}

