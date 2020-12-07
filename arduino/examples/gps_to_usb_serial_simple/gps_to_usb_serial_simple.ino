#include "teanc_core.h"


// TeaNC core configuration:
TeancPeripheralsCfg peripheralsCfg = {
  enableGPS: true,
  enableVHF: true,
  enableLCD: false,
};
TeaNC teanc(peripheralsCfg);


// Setup radio configuration:
FiltCfg filtcfg = {
  enableEmph:   RADIO_FILT_BYPASS,
  enableHPF:    RADIO_FILT_BYPASS,
  enableLPF:    RADIO_FILT_BYPASS
};

TransceiverCfg transceiverCfg = {
  bandwidth:  RADIO_BANDWIDTH_12500_HZ,
  txFreq:     144.600,
  rxFreq:     144.600,
  txSubtone:  RADIO_CTCS_DISABLE,
  rxSubtone:  RADIO_CTCS_DISABLE,
  squelch:    0,
};



void setup() {    
  SerialUSB->begin(115200);      
  SerialUSB->println("ESP32 GPS and VHF testing");

  teanc.begin();

  SerialUSB->println("Turning on VHF radio");
  VHF.setup(transceiverCfg, filtcfg);
  VHF.powerOn();

  digitalWrite(USER_LED_PIN, HIGH);
    
  SerialUSB->println("SA818 Radio Version");
  VHF.getVer();  
  
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
