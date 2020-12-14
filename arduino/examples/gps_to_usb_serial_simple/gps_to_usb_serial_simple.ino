#include "teanc_core.h"
// This example demonstrates basic GPS reception,
// relaying GPS messages straight from the NEO-6
// GPS receiver to the USB serial device.


// TeaNC core configuration:
TeancPeripheralsCfg peripheralsCfg = {
  enableGPS: true,
  enableVHF: false,
  enableLCD: false,
};
TeaNC teanc(peripheralsCfg);



void setup() {    
  SerialUSB->begin(115200);      
  SerialUSB->println("ESP32 simple GPS receiver");

  teanc.begin();

  digitalWrite(USER_LED_PIN, HIGH);
  
  delay(1000);
}

void loop() {
  char temp;
  
  while(SerialGPS->available()){
    temp = SerialGPS->read();
    SerialUSB->print(temp);
  }
  if(digitalRead(USER_PUSH_BTN1_PIN) == HIGH){
    digitalWrite(USER_LED_PIN, HIGH);
  }
  else if(digitalRead(USER_PUSH_BTN2_PIN) == HIGH){
    digitalWrite(USER_LED_PIN, HIGH);
  }
  else{
    digitalWrite(USER_LED_PIN, LOW);
  }
  
}
