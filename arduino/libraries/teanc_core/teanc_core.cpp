#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "teanc_core.h"
#include "sa818v.h"


# define TEANC_WIFI_JOIN_TIMEOUT 10   //sec


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
  
  
  
// Constructors:
TeaNC::TeaNC() {}
TeaNC::~TeaNC() {}

TeaNC::TeaNC(TeancPeripheralsCfg peripheralsCfg) {
  this->peripheralsCfg = peripheralsCfg; 
}

TeaNC::TeaNC(WifiCfg wifiCfg) {
  this->wifi.preferredCfg = wifiCfg;
  this->wifi.enable = true;
}

TeaNC::TeaNC(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiCfg) {
  this->peripheralsCfg = peripheralsCfg; 
  this->wifi.preferredCfg = wifiCfg;
  this->wifi.enable = true;
}

TeaNC::TeaNC(WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg) {
  this->wifi.preferredCfg = wifiPreferredCfg;
  this->wifi.fallbackCfg = wifiFallbackCfg;
  this->wifi.fallbackDefined = true;
  this->wifi.enable = true;
}

TeaNC::TeaNC(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg) {
  this->peripheralsCfg = peripheralsCfg; 
  this->wifi.preferredCfg = wifiPreferredCfg;
  this->wifi.fallbackCfg = wifiFallbackCfg;
  this->wifi.fallbackDefined = true;
  this->wifi.enable = true;
}



// Start methods:

void TeaNC::begin(TeancPeripheralsCfg peripheralsCfg) {
  this->peripheralsCfg = peripheralsCfg;
  this->begin();
}

void TeaNC::begin(WifiCfg wifiCfg) {
  this->wifi.preferredCfg = wifiCfg;
  this->wifi.enable = true;
  this->begin();
}

void TeaNC::begin(WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg) {
  this->wifi.preferredCfg = wifiPreferredCfg;
  this->wifi.fallbackCfg = wifiFallbackCfg;
  this->wifi.fallbackDefined = true;
  this->wifi.enable = true;
  this->begin();
}

void TeaNC::begin(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiCfg) {
  this->peripheralsCfg = peripheralsCfg;
  this->wifi.preferredCfg = wifiCfg;
  this->wifi.enable = true;
  this->begin();
}

void TeaNC::begin(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg) {
  this->peripheralsCfg = peripheralsCfg;
  this->wifi.preferredCfg = wifiPreferredCfg;
  this->wifi.fallbackCfg = wifiFallbackCfg;
  this->wifi.fallbackDefined = true;
  this->wifi.enable = true;
  this->begin();
}


void TeaNC::begin() {
  
  // Initialize Wifi (if enabled):
  if(this->wifi.enable){
    this->startWifi();
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


bool TeaNC::tryWifiCfg(WifiCfg cfg) {  
  unsigned long tstart;
  bool connected = false;
  if(cfg.mode == WIFI_MODE_STA) {
    SerialUSB->println("");
    SerialUSB->printf("Connecting to SSID \"%s\"\n", cfg.ssid);
    WiFi.begin(cfg.ssid, cfg.password);
    tstart = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      if((millis()-tstart)/1000 > TEANC_WIFI_JOIN_TIMEOUT){
        SerialUSB->printf("Timed out trying to join \"%s\"\n", cfg.ssid); 
        break;
      }
    }
    
    if(WiFi.status() == WL_CONNECTED) {
      SerialUSB->print("Wifi connected. IP: ");
      SerialUSB->println(WiFi.localIP());
      connected = true;
    }
    
  } else if(cfg.mode == WIFI_MODE_AP) {
    if(strlen(cfg.password)>0){
      SerialUSB->printf("Starting AP with SSID \"%s\" with password.\n", cfg.ssid);
      WiFi.softAP(cfg.ssid, cfg.password);
    } else {
      SerialUSB->printf("Starting AP with SSID \"%s\" without password.\n", cfg.ssid);
      WiFi.softAP(cfg.ssid);
    }
    IPAddress myIP = WiFi.softAPIP();
    SerialUSB->print("AP IP address: ");
    SerialUSB->println(myIP);
    connected = true;
  }
  
  return connected;
}


void TeaNC::startWifi() {
  bool connected = this->tryWifiCfg(this->wifi.preferredCfg);
  if(!connected && this->wifi.fallbackDefined){
    SerialUSB->println("Trying fallback wifi cfg..");
    connected = this->tryWifiCfg(this->wifi.fallbackCfg);    
  }
}