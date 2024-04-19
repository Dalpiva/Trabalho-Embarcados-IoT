#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include "boards.h"

#define QTD_SENSORES 10
float pacote[QTD_SENSORES + 2];

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

void loop()
{
  int packet_size = LoRa.parsePacket();

  if (packet_size > 0) 
  {
    // Lê os bytes recebidos e os armazena em pacote
    int read = LoRa.readBytes((uint8_t *)pacote, sizeof(pacote));

    if(pacote[0] == 2.0)
    {
      pacote[0] = 3;
      for(int i = 0; i < QTD_SENSORES + 2 ; i++)
      {
        if(i == 11){
          Serial.print(pacote[i]);
        }else{
          Serial.println(pacote[i]);
        }
        delay(5);
      }
    }
  }
}