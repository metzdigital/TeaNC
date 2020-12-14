#include "teanc_core.h"
//This example turns on the VHF radio and GPS, and connects to wifi,
//just to demonstrate how to set things up.

//Either remove this line and use the subsequent #defines instead,
//or write your own wifi_private.h file containing these #defines.
#include "wifi_private.h"
//const char wifi_ssid[] = "my_wifi_ssid";
//const char wifi_password[] = "my_wifi_password";


// TeaNC core configuration:
TeancPeripheralsCfg peripheralsCfg = {
  enableGPS: true,
  enableVHF: true,
  enableLCD: false,
};

WifiCfg wifiCfg = {
  mode:      WIFI_MODE_STA,  // WIFI_MODE_STA or WIFI_MODE_AP
  ssid:      wifi_ssid,
  password:  wifi_password,
};

WifiCfg fallbackWifiCfg = {
  mode:      WIFI_MODE_AP,  // WIFI_MODE_STA or WIFI_MODE_AP
  ssid:      "teanc",       // Access point SSID
  password:  "",            // leave as blank "" for open
};

TeaNC teanc(peripheralsCfg, wifiCfg, fallbackWifiCfg);


// Radio configuration:
TransceiverCfg transceiverCfg = {
  bandwidth:  RADIO_BANDWIDTH_12500_HZ,
  txFreq:     144.600,
  rxFreq:     144.600,
  txSubtone:  RADIO_CTCS_DISABLE,
  rxSubtone:  RADIO_CTCS_DISABLE,
  squelch:    0,
};

FiltCfg filtcfg = {
  enableEmph:   RADIO_FILT_BYPASS,
  enableHPF:    RADIO_FILT_ENABLE,
  enableLPF:    RADIO_FILT_ENABLE
};



void setup() {    
  SerialUSB->begin(115200);      
  SerialUSB->println("ESP32 GPS and VHF testing");

  //Start TeaNC core:   
  teanc.begin(); //initializes pins and GPS and VHF serial objects

  //Turn on the radio:
  SerialUSB->println("Turning on VHF radio");
  VHF.setup(transceiverCfg, filtcfg);
  VHF.powerOn();
  SerialUSB->println("SA818 Radio Version");
  VHF.getVer();  

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
