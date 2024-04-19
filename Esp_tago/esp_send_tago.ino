float pacote[12];
float normal; 

void setup() {
  Serial.begin(115200);
}

void loop() {
  if(Serial.available()) {
    normal = Serial.parseFloat(); 
    Serial.println(normal);
  }
}