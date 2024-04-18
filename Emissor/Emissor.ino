#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
/* Definicoes para comunicação com radio LoRa */
/*      Pinos LoRa          Pinos ESP32        */
#define SCK_LORA            5
#define MISO_LORA           19
#define MOSI_LORA           27
#define RESET_PIN_LORA      14
#define SS_PIN_LORA         18
 
#define HIGH_GAIN_LORA      20  /* dBm */
#define BAND                900E6  /* 915MHz de frequencia */

#define SENSOR_1            4
#define QTD_SENSORES        10 + 2

/* Definicoes gerais */
#define DEBUG_SERIAL_BAUDRATE    115200
 
/* Local prototypes */
bool init_comunicacao_lora(void);
 
/* Funcao: inicia comunicação com chip LoRa
 * Parametros: nenhum
 * Retorno: true: comunicacao ok
 *          false: falha na comunicacao
*/

/* Sensoreamento */
OneWire oneWire(SENSOR_1);
DallasTemperature sensors(&oneWire);

/* Variaveis Globais*/

// QTD SENSORES + ID + RATO OU PEIXE
float pacote[QTD_SENSORES + 2];

DeviceAddress sensor1 = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x20, 0xF5, 0x4E };
DeviceAddress sensor2 = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0xE , 0xB2, 0x96 };

bool init_comunicacao_lora(void)
{
  bool status_init = false;
  Serial.println("[LoRa Sender] Tentando iniciar comunicacao com o radio LoRa...");
  SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
  LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
    
  if (!LoRa.begin(BAND)) 
  {
      Serial.println("[LoRa Sender] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
      delay(1000);
      status_init = false;
  }
  else
  {
      /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */
      LoRa.setTxPower(HIGH_GAIN_LORA); 
      Serial.println("[LoRa Sender] Comunicacao com o radio LoRa ok");
      status_init = true;
  }

  return status_init;
}
 
/* Funcao de setup */
void setup() 
{    
  Serial.begin(DEBUG_SERIAL_BAUDRATE);
  while (!Serial);

  /* Tenta, até obter sucesso, comunicacao com o chip LoRa */
  while(init_comunicacao_lora() == false);       

  sensors.begin();
}
 
/* Programa principal */
void loop() 
{
  // Recebe todas as temperatuas do sensor
  sensors.requestTemperatures(); 

  // Adiciona as temperaturas no vetor de envio
  /*temperaturas[0] = sensors.getTempCByIndex(0);
  temperaturas[1] = sensors.getTempFByIndex(0);*/

  pacote[0] = 1.0; // ID DO BIOTERIO
  pacote[1] = 0.0; // ID 0 = PEIXE // 1 = RATO

  pacote[2] = sensors.getTempC(sensor1);
  pacote[3] = sensors.getTempC(sensor2);
  pacote[4] = sensors.getTempF(sensor1);
  pacote[5] = sensors.getTempF(sensor2);

  // Envia os dados
  LoRa.beginPacket();
  LoRa.write((uint8_t *)pacote, sizeof(pacote));
  LoRa.endPacket();

  // Printa na tela os dados
  for(int i = 0; i < QTD_SENSORES ; i++)
  {
    Serial.println(pacote[i]);
  }
  Serial.println("----------");

  delay(5000);
}

