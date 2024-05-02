// ------------------------------------------------------------------------------ //
// BIBLIOTECAS                                                                    //
// ------------------------------------------------------------------------------ //
// Includes gerais
#include <stdio.h>
#include <stdlib.h>

// Sensores
#include <OneWire.h>
#include <DallasTemperature.h>

// SD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// LORA
#include <LoRa.h>

// ------------------------------------------------------------------------------ //
// DEFINES                                                                        //
// ------------------------------------------------------------------------------ //
// Padroes
#define BAUDRATE            115200
#define DELAY               15000     // 15.000 MS = 15 SEGUNDOS

// Sensores
#define QTD_SENSORES        10 + 2    // 10 SENSORES + 2 IDs
#define SENSOR_1_PIN        4
#define SENSOR_2_PIN        2

// SPI
#define SCK_LORA            18
#define MISO_LORA           19
#define MOSI_LORA           23
#define RESET_PIN_LORA      14

// SD
#define CS_PIN_SD_CARD      5

// LORA
#define CS_PIN_LORA         15
#define HIGH_GAIN_LORA      20        // dBm
#define BAND                900E6     // 900MHz de frequencia

// Bateria
#define BATTERY_PIN_1       25
#define BATTERY_PIN_2       26
#define BATTERY_PIN_3       27

// ------------------------------------------------------------------------------ //
// VARIAVEIS                                                                      //
// ------------------------------------------------------------------------------ //
// Sensores
float pacote_de_dados[QTD_SENSORES];

OneWire oneWire_1(SENSOR_1_PIN);
OneWire oneWire_2(SENSOR_2_PIN);

DallasTemperature sensors_1(&oneWire_1);
DallasTemperature sensors_2(&oneWire_2);

DeviceAddress sensor1  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x20, 0xF5, 0x4E };
DeviceAddress sensor2  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0xE , 0xB2, 0x96 };
DeviceAddress sensor3  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x20, 0xF5, 0x4E };
DeviceAddress sensor4  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0xE , 0xB2, 0x96 };
DeviceAddress sensor5  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x20, 0xF5, 0x4E };
DeviceAddress sensor6  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0xE , 0xB2, 0x96 };
DeviceAddress sensor7  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x20, 0xF5, 0x4E };
DeviceAddress sensor8  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0xE , 0xB2, 0x96 };
DeviceAddress sensor9  = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0x20, 0xF5, 0x4E };
DeviceAddress sensor10 = { 0x28, 0xFF, 0x64, 0x18, 0x99, 0xE , 0xB2, 0x96 };

// Bateria
bool energia = true;
int status_energia = 1;
bool status_sincronia = false; 

// SD
int max_recuperar = 5;
int passo = 0;
bool lock = false;

char buffer[10];
float salvo[12];
int _index = 0;
int _jndex = 1;


// ------------------------------------------------------------------------------ //
// FUNCOES - Temperatura                                                          //
// ------------------------------------------------------------------------------ //
void request_temperatures(void)
{
  pacote_de_dados[0] = 1.0; // ID DO BIOTERIO
  pacote_de_dados[1] = 0.0; // ID 0 = PEIXE // 1 = RATO

  sensors_1.requestTemperatures();

  pacote_de_dados[2]  = sensors_1.getTempC(sensor1);
  pacote_de_dados[3]  = sensors_1.getTempC(sensor2);
  pacote_de_dados[4]  = sensors_1.getTempC(sensor3);
  pacote_de_dados[5]  = sensors_1.getTempC(sensor4);
  pacote_de_dados[6]  = sensors_1.getTempC(sensor5);

  sensors_2.requestTemperatures();

  pacote_de_dados[7]  = sensors_2.getTempC(sensor6);
  pacote_de_dados[8]  = sensors_2.getTempC(sensor7);
  pacote_de_dados[9]  = sensors_2.getTempC(sensor8);
  pacote_de_dados[10] = sensors_2.getTempC(sensor9);
  pacote_de_dados[11] = sensors_2.getTempC(sensor10);
}

// ------------------------------------------------------------------------------ //
// FUNCOES - LoRa                                                                 //
// ------------------------------------------------------------------------------ //
bool init_comunicacao_lora(void)
{
  bool status_init = false;
  SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, CS_PIN_LORA);
  LoRa.setPins(CS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
    
  if (!LoRa.begin(BAND)) 
  {
    Serial.println("Comunicacao com o radio LoRa falhou.");        
    delay(1000);
    status_init = false;
  }
  else
  {
    /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */
    LoRa.setTxPower(HIGH_GAIN_LORA); 
    status_init = true;
  }

  return status_init;
}

void envia_backup(char dado)
{
  if((dado == ';') || (dado == '\n'))
  {
    salvo[_jndex] = strtof(buffer, NULL);

    if (buffer[0] == '-')
    {
      salvo[_jndex] = -salvo[_jndex];
    }
    _index = 0;
    _jndex++;
  }
  else
  {
    buffer[_index] = dado;
    _index++;
  }   


  if(dado == '\n')
  {
    salvo[0] = 1.0;
    LoRa.beginPacket();
    LoRa.write((uint8_t *)salvo, sizeof(salvo));
    LoRa.endPacket();  
    _jndex = 1;
  }

  return;
}

// ------------------------------------------------------------------------------ //
// FUNCOES - SD Card                                                              //
// ------------------------------------------------------------------------------ //
bool cria_arquivo(fs::FS &fs, const char * path)
{
  File file = fs.open(path, FILE_WRITE);

  if(!file)
  {
    Serial.println("Falha ao abrir o arquivo!");
    return false;
  }
 
  file.close();
  return true;
}

bool salva_dado(fs::FS &fs, const char * path, float sensores[])
{
  File file = fs.open(path, FILE_APPEND);

  if(!file)
  {
    Serial.println("Falha ao abrir o arquivo para append!");
    return false;
  }

  String string_de_dados = "";
  for(int i = 0; i < QTD_SENSORES-2; i++)
  {
    if(i > 0) string_de_dados += ";";  
    
    string_de_dados += String(sensores[i]);
  }

  string_de_dados += "\n";

  if(!(file.print(string_de_dados)))
  {
    Serial.println("Adicao de novos dados no arquivo falhou!");
    return false;
  } 

  file.close();
  return true;
}

bool le_arquivo(fs::FS &fs, const char * path)
{
  char arq; 
  int recuperar = 0;

  File file = fs.open(path);
  if(!file)
  {
    Serial.println("Falha ao abrir o arquivo para leitura!");
    return false;
  }
  
  while(file.available())
  {
    arq = file.read();

    if(recuperar >= passo*5)
    {
      if(recuperar < max_recuperar)
      {
        envia_backup(arq);
      }
      else
      {
        lock = true;
      }
    }

    if(arq == '\n')
    {
      recuperar++;
      Serial.println(recuperar);
    }
  } 

  file.close();

  if(lock)
  {
    Serial.println("Finalizou nao terminando!");
    max_recuperar += 5;
    lock = false;
    passo++;
    return false;  
  }

  
  Serial.println("Finalizou terminando!");
  max_recuperar = 5;
  return true;  
}

void deleta_arquivo(fs::FS &fs, const char * path)
{
  if(fs.remove(path))
  {
    Serial.println("Arquivo deletado!");
  } 
}

bool init_cartao_sd(void)
{
  if(!SD.begin(CS_PIN_SD_CARD))
  {
    Serial.println("Munt do Cartao Falhou!");
    delay(1000);
    return false;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE)
  {
    Serial.println("Nenhum cartao inserido!");
    delay(1000);
    return false;
  }

  cria_arquivo(SD, "/backup_bioterio.csv");

  return true;
}

// ------------------------------------------------------------------------------ //
// FUNCOES - Bateria                                                              //
// ------------------------------------------------------------------------------ //
bool verifica_energia(void)
{
  int pino_1 = digitalRead(BATTERY_PIN_1);
  //int pino_2 = digitalRead(BATTERY_PIN_2);
  //int pino_3 = digitalRead(BATTERY_PIN_3);

  if (pino_1 == 1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

// ------------------------------------------------------------------------------ //
// SETUP                                                                          //
// ------------------------------------------------------------------------------ //
void setup() 
{
  Serial.begin(BAUDRATE);
  while(!Serial);

  while(init_cartao_sd() == false);
  delay(1000);

  while(init_comunicacao_lora() == false);
  delay(1000);

  pinMode(BATTERY_PIN_1, INPUT);
  pinMode(BATTERY_PIN_2, INPUT);
  pinMode(BATTERY_PIN_3, INPUT);
  delay(100);

  Serial.println("Sucesso em iniciar os perifericos!");

  sensors_1.begin();
  sensors_2.begin();
}

// ------------------------------------------------------------------------------ //
// LOOP                                                                           //
// ------------------------------------------------------------------------------ //
void loop() 
{
  request_temperatures();

  energia = verifica_energia();

  if(energia && status_energia == 0)
  {
    bool finalizado = le_arquivo(SD, "/backup_bioterio.csv");

    if(finalizado)
    {
      deleta_arquivo(SD, "/backup_bioterio.csv");
      cria_arquivo(SD, "/backup_bioterio.csv");
      status_energia = 1;
      Serial.println("Indo para o lora");
    }
  }
  else if (energia)
  {
    LoRa.beginPacket();
    LoRa.write((uint8_t *)pacote_de_dados, sizeof(pacote_de_dados));
    LoRa.endPacket();  

    Serial.println("Dados enviados pelo LoRa!");
  }
  else
  {
    float temperaturas[10];
    
    for(int i = 0; i < QTD_SENSORES - 1; i++)
    {
      temperaturas[i] = pacote_de_dados[i + 1];
    }

    salva_dado(SD, "/backup_bioterio.csv", temperaturas);

    Serial.println("Dados salvos no cartao SD!");

    status_energia = 0;
    max_recuperar = 5;
    passo = 0;
  }

  
  delay(DELAY);
}
