#include <Arduino.h>
#include <WiFi.h>
#include "teanc_core.h"
#include "sa818v.h"


//Make serial interface pointers:
extern HardwareSerial* SerialUSB = &Serial;
extern HardwareSerial* SerialGPS = &Serial1;
extern HardwareSerial* SerialVHF = &Serial2;


//Make radio config:
extern RadioInterfaces radioInterfaces = {
  powerEnablePin:  VHF_PWR_DOWN_PIN,
  radioModePin:    VHF_RX_TXN_PIN,
  RadioSerial:     SerialVHF,
  LogSerial:       SerialUSB,
};
extern Radio VHF(radioInterfaces);
  
  
  

TeaNC::TeaNC() {}
TeaNC::~TeaNC() {}

TeaNC::TeaNC(TeancPeripheralsCfg peripheralsCfg) {
  this->peripheralsCfg = peripheralsCfg; 
}

TeaNC::TeaNC(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiCfg) {
  this->peripheralsCfg = peripheralsCfg; 
  this->wifiCfg = wifiCfg;
}

TeaNC::TeaNC(WifiCfg wifiCfg) {
  this->wifiCfg = wifiCfg;
}


void TeaNC::begin(TeancPeripheralsCfg peripheralsCfg) {
  this->peripheralsCfg = peripheralsCfg;
  this->begin();
}

void TeaNC::begin(WifiCfg wifiCfg) {
  this->wifiCfg = wifiCfg;
  this->begin();
}

void TeaNC::begin(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiCfg) {
  this->peripheralsCfg = peripheralsCfg;
  this->wifiCfg = wifiCfg;
  this->begin();
}

void TeaNC::begin() {
  
  // Initialize Wifi (if enabled):
  if(this->wifiCfg.enableWifi){
    WiFi.begin(this->wifiCfg.wifiSSID, this->wifiCfg.wifiPassword);
    SerialUSB->println("");
    SerialUSB->print("Connecting to Wifi..");
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      SerialUSB->print(".");
    }
    SerialUSB->print("Wifi connected. IP: ");
    SerialUSB->println(WiFi.localIP());        
  }  
  
  // Initialize GPS (if enabled):
  if(this->peripheralsCfg.enableGPS){
    //Setup the GPS pin modes
    pinMode(GPS_UART_TX_PIN, OUTPUT);
    pinMode(GPS_UART_RX_PIN, INPUT);
    pinMode(GPS_PPS_PIN, INPUT);
    Serial1.begin(9600, SERIAL_8N1, GPS_UART_RX_PIN, GPS_UART_TX_PIN);
  }  
  
  // Initialize VHF (if enabled):  
  if(this->peripheralsCfg.enableVHF){    
    //Setup the VHF pin modes
    pinMode(VHF_UART_TX_PIN, OUTPUT);
    pinMode(VHF_UART_RX_PIN, INPUT);
    pinMode(VHF_RX_TXN_PIN, OUTPUT);
    pinMode(VHF_PWR_DOWN_PIN, OUTPUT);
    Serial2.begin(RADIO_SERIAL_BAUD, SERIAL_8N1, VHF_UART_RX_PIN, VHF_UART_TX_PIN);
  }
  
	//User push buttons and LED
	pinMode(USER_LED_PIN, OUTPUT);
	pinMode(USER_PUSH_BTN1_PIN, INPUT);
	pinMode(USER_PUSH_BTN2_PIN, INPUT);

	//Display pin modes
	pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
	 
}