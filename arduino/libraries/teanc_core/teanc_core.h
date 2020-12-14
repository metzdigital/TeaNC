#ifndef TEANC_CORE_H_
#define TEANC_CORE_H_

#include "Arduino.h"
#include "sa818v.h"

#include <IPv6Address.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include <driver/i2s.h>
#include <driver/adc.h>



//Return types
typedef enum{
	SUCCESS=0,
	FAILURE,
	TIMEOUT
} RetStatus_t;

//User related IO assignments
#define USER_LED_PIN 5
#define USER_PUSH_BTN1_PIN 39
#define USER_PUSH_BTN2_PIN 34

//Power sources voltage sense pin
#define PWR_SENSE_V_PIN 4

//Display IO Assignments 
#define DISPLAY_ENABLE_PIN 19
#define DISPLAY_I2C_SCL_PIN 22
#define DISPLAY_I2C_SDA_PIN 21

//GPS IO Assignments 
#define GPS_UART_RX_PIN 27
#define GPS_UART_TX_PIN 26
#define GPS_PPS_PIN 35

//VHF IO Assignments 
#define VHF_UART_RX_PIN 33
#define VHF_UART_TX_PIN 32
#define VHF_PWR_DOWN_PIN 13
#define VHF_RX_TXN_PIN 14
#define VHF_AOUT_PIN 25
#define VHF_AIN_PIN 36

//Serial object pointers
extern HardwareSerial* SerialUSB;
extern HardwareSerial* SerialGPS;
extern HardwareSerial* SerialVHF;

//Make radio interfaces:
extern RadioInterfaces radioInterfaces;
extern Radio VHF;

typedef struct {
  uint16_t sample_rate; // samples per sec
  uint16_t buffer_len;  // samples
} AudioFromRadioCfg;


typedef struct {
  bool enableGPS;
  bool enableVHF;
  bool enableLCD;  
} TeancPeripheralsCfg;



typedef struct {
  wifi_mode_t mode;
  const char* ssid;
  const char* password;
} WifiCfg;
  
typedef enum {
  WIFI_CFG_NONE,
  WIFI_CFG_PREFERRED,
  WIFI_CFG_FALLBACK
} WifiSelectCfg;



class TeaNC {
  public: 
    TeaNC();
    TeaNC(TeancPeripheralsCfg peripheralsCfg);
    TeaNC(WifiCfg wifiCfg);
    TeaNC(WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg);
    TeaNC(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiCfg);
    TeaNC(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg);
    ~TeaNC();
    
    void begin();
    void begin(TeancPeripheralsCfg peripheralsCfg);
    void begin(WifiCfg wifiCfg);
    void begin(WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg);
    void begin(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiCfg);
    void begin(TeancPeripheralsCfg peripheralsCfg, WifiCfg wifiPreferredCfg, WifiCfg wifiFallbackCfg);
    
    
  private:
    TeancPeripheralsCfg peripheralsCfg = {
      enableGPS: true,
      enableVHF: true,
      enableLCD: false,
    };
    
    AudioFromRadioCfg audioFromRadioCfg = {
      sample_rate: 32768,
      buffer_len:  1024,
    };

    struct {
      bool enable = false;
      WifiCfg preferredCfg;
      WifiCfg fallbackCfg;
      bool fallbackDefined = false;    
      WifiSelectCfg activeCfg = WIFI_CFG_NONE;
      wifi_mode_t mode;
    } wifi;
    
    void startWifi();
    bool tryWifiCfg(WifiCfg cfg);

    
    TaskHandle_t audioSamplingTask;
    uint16_t* audio_in_buffer;  // need to calloc space for this in start method

    
};

#endif 