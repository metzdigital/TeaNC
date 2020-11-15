#include "Arduino.h"
#include "sa818v.h"
#include <assert.h>


Radio::Radio() {}
Radio::~Radio() {}


// Establish serial connection to SA818 Radio command interface
// This requires a serial stream object be attached to the Radio object in your main sketch.
// Ex:  radio.setControlSerial(&Serial1);
void Radio::connect()
{
  assert(controlSerialAttached);
  serialTxLen = sprintf(serialTxBuf, "%s\r\n", RADIO_CMD_CONNECT);
  sendCmd();
  //fixme: verify expected response
}

void Radio::setConfig(RadioCfg cfg)
{
  serialTxLen = sprintf(serialTxBuf, "%s=%0i,%3.4f,%3.4f,%4s,%i,%4s\r\n", RADIO_CMD_SET_GROUP, cfg.bandwidth, cfg.txf, cfg.rxf, cfg.tx_subaudio, cfg.squelch, cfg.rx_subaudio);
  radioCfg = cfg;
  sendCmd();
}


// Sets key radio control parameters
void Radio::setConfig(uint8_t bandwidth, double txf, double rxf, char* tx_subaudio, uint8_t squelch, char* rx_subaudio)
{
  serialTxLen = sprintf(serialTxBuf, "%s=%0i,%3.4f,%3.4f,%4s,%i,%4s\r\n", RADIO_CMD_SET_GROUP, bandwidth, txf, rxf, tx_subaudio, squelch, rx_subaudio);
  sendCmd();
  //fixme: verify expected response

  // make note of this config:
  radioCfg.bandwidth = bandwidth;
  radioCfg.txf = txf;
  radioCfg.rxf = rxf;
  radioCfg.tx_subaudio = tx_subaudio;
  radioCfg.rx_subaudio = rx_subaudio;
  radioCfg.squelch = squelch;
}


// Checks for signal received on specified frequency
void Radio::scanFreq(double rxf)
{
  serialTxLen = sprintf(serialTxBuf, "%s+%3.4f\r\n", RADIO_CMD_SCAN_FREQ, rxf);
  sendCmd();
  //fixme: parse response
}


// Sets volume of received audio
void Radio::setVolume(uint8_t vol)
{
  serialTxLen = sprintf(serialTxBuf, "%s=%0i\r\n", RADIO_CMD_SET_VOL, vol);
  sendCmd();
  //fixme: verify expected response
}



// Enables or bypasses pre/de-emphasis filter, high-pass filter, and low-pass filter
void Radio::setFilter(FiltCfg cfg){
  serialTxLen = sprintf(serialTxBuf, "%s=%0i,%0i,%0i\r\n", RADIO_CMD_SET_FILT, 
    (cfg.enableEmph ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS),
    (cfg.enableHPF  ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS),
    (cfg.enableLPF  ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS)
  );
  sendCmd();
  //fixme: verify expected response
  filtCfg = cfg;
}


// Enables or bypasses pre/de-emphasis filter, high-pass filter, and low-pass filter
void Radio::setFilter(bool enableEmph, bool enableHPF, bool enableLPF)
{
  serialTxLen = sprintf(serialTxBuf, "%s=%0i,%0i,%0i\r\n", RADIO_CMD_SET_FILT, 
    (enableEmph ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS),
    (enableHPF  ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS),
    (enableLPF  ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS)
  );
  sendCmd();
  //fixme: verify expected response

  filtCfg.enableEmph = enableEmph;
  filtCfg.enableLPF = enableLPF;
  filtCfg.enableHPF = enableHPF;
}




// Opens or closes radio tail tone
void Radio::setTail(bool openTail)
{
  serialTxLen = sprintf(serialTxBuf, "%s=%0i\r\n", RADIO_CMD_SET_TAIL, openTail);
  sendCmd();
  //fixme: verify expected response
}


// Evaluates radio signal stength indication  
uint8_t Radio::getSignalStrength()
{
  unsigned int rssi = 0;
  serialTxLen = sprintf(serialTxBuf, "%s?\r\n", RADIO_CMD_GET_RSSI);
  sendCmd();
  //fixme: parse response
  if(serialRxLen>0){
    sscanf(serialRxBuf, "RSSI=%03u", &rssi);
  }
  return (uint8_t)rssi;
}


// Queries radio module firmware version
void Radio::getVer()
{
  serialTxLen = sprintf(serialTxBuf, "%s?\r\n", RADIO_CMD_GET_VER);
  sendCmd();
  //fixme: parse response
}


void Radio::sendCmd()
{
  assert(controlSerialAttached);
  #ifdef RADIO_DEBUG
    assert(logSerialAttached);
    LogSerial->print("To radio: ");
    LogSerial->print(serialTxBuf);
  #endif
  ControlSerial->flush();
  // Send cmd to radio:
  ControlSerial->print(serialTxBuf);
  delay(1);
  // Receive response from radio cmd interface:
  receiveReply();
  #ifdef RADIO_DEBUG
    assert(logSerialAttached);
    LogSerial->print("From radio: ");
    LogSerial->println(serialRxBuf);
  #endif
}



// Waits for acknowledgement from SA818 command interface
void Radio::receiveReply()
{
  unsigned char readByte;
  
  // wait here until receive byte
  // fixme:  add timeout here
  while(!ControlSerial->available()){
    delay(1);
  }

  serialRxLen = 0;
    
  // Collect bytes until EOL or reach max buffer length:
  while(ControlSerial->available()){

    // read one byte:
    readByte = ControlSerial->read();
    
    // check if it's end of line:
    if(readByte=='\r' || readByte=='\n'){
      serialRxBuf[serialRxLen]='\0';
      return;
 
    // if receiving then receive:
    } else if(serialRxLen<RADIO_SERIAL_RX_BUF_LEN) {
      serialRxBuf[serialRxLen++] = readByte;

    // if reached max allowable msg length then dump and reset buffer:
    } else if(serialRxLen>=RADIO_SERIAL_RX_BUF_LEN) {
      LogSerial->println("Error:  radio cmd reply exceeds buffer length.");
      serialRxLen = 0;
      return;
    }
  }
}
