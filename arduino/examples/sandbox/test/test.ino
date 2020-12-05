
#include "../../../src/config.h"
#include "../../../src/sa818/sa818v.h"


HardwareSerial *SerialUSB;
HardwareSerial *SerialGPS;
HardwareSerial *SerialVHF;

void setup() {
  //setup IO that doesn't have a lib yet
  configIO();
  
  SerialUSB = &Serial;
  SerialGPS = &Serial1;
  SerialVHF = &Serial2;
    
  SerialUSB->begin(115200);
  SerialGPS->begin(9600, SERIAL_8N1, GPS_UART_RX_PIN, GPS_UART_TX_PIN);

  SerialUSB->println("ESP32 GPS testing");
  digitalWrite(USER_LED_PIN, HIGH);
  
  Radio vhf(&Serial2);
  vhf.setLogSerial(&Serial);
  SerialUSB->println("SA818 Connect to VHF");
  vhf.connect();
  
  Serial.println("SA818 Radio Version");
  vhf.getVer();
  
  struct RadioCfg cfg = {
    bandwidth:    RADIO_BANDWIDTH_12500_HZ,
    txf:          145.000,
    rxf:          145.000,
    tx_subaudio:  RADIO_CTCS_DISABLE,
    squelch:      0,
    rx_subaudio:  RADIO_CTCS_DISABLE
  };
  
  SerialUSB->println("SA818 Config VHF");
  vhf.setConfig(cfg);

  SerialUSB->println("SA818 Config Filters");
  struct FiltCfg fcfg = {
    enableEmph:   RADIO_FILT_BYPASS,
    enableHPF:    RADIO_FILT_BYPASS,
    enableLPF:    RADIO_FILT_BYPASS
  };
  vhf.setFilter(fcfg);
  
  delay(1000);

}

void loop() {
  char temp;
  uint8_t bytes; 
  static uint8_t val=0;

  
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
