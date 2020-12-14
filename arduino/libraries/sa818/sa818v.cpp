#include <Arduino.h>
#include "sa818v.h"
#include <assert.h>

/**********************
The Radio object defined here requires interfaces be defined prior to starting.
Interfaces may be defined either when the object is created or during setup.
************************/


// CONSTRUCTORS:
Radio::Radio() {}
Radio::~Radio() {}

Radio::Radio(RadioInterfaces interfaces) {
  this->interfaces = interfaces;
  this->interfacesDefined = true;
}

void Radio::setup(RadioInterfaces interfaces) {
  this->interfaces = interfaces;
  this->interfacesDefined = true;
}

void Radio::setup(TransceiverCfg transceiverCfg) {
  this->transceiverCfg = transceiverCfg;
  this->interfacesDefined = true;
}

void Radio::setup(RadioInterfaces interfaces, TransceiverCfg transceiverCfg) {
  this->interfaces = interfaces;
  this->transceiverCfg = transceiverCfg;
  this->interfacesDefined = true;
}

void Radio::setup(RadioInterfaces interfaces, TransceiverCfg transceiverCfg, FiltCfg filtcfg) {
  this->interfaces = interfaces;
  this->transceiverCfg = transceiverCfg;
  this->filtCfg = filtcfg;
  this->interfacesDefined = true;
}

void Radio::setup(TransceiverCfg transceiverCfg, FiltCfg filtcfg) {
  this->transceiverCfg = transceiverCfg;
  this->filtCfg = filtcfg;
  this->interfacesDefined = true;
}


// Power on and initialize the radio:
void Radio::powerOn(){
  
  // Require interfaces started by Radio::setup() or in the constructor
  assert(this->interfacesDefined);
  
	// Set tx/rx control pin to put the radio in rx mode when it starts:
	this->receiveMode();
  
	// Turn on the radio:
	digitalWrite(this->interfaces.powerEnablePin, RADIO_PWR_ON_STATE);
	delay(100);  //Incase the radio was in power down state give some time to boot
	this->isPowered = true;
  
  // Initiate serial connection:
  this->radioSerialTxLen = sprintf(this->radioSerialTxBuf, "%s\r\n", RADIO_CMD_CONNECT);
  this->sendCmd();
  //fixme: verify expected response
  
  // Send transceiver config:
  this->sendTransceiverCfg();
  
  // Send radio filters config:
  this->sendFiltCfg();
    

}
  
  
//put the radio in a lower power state
void Radio::powerOff(){
	digitalWrite(this->interfaces.powerEnablePin, RADIO_PWR_OFF_STATE);
	this->isPowered = false;
}
  
  
// Set the transceiver config in local memory, and
// command the radio to this config if the radio is on:
void Radio::setTransceiverCfg(TransceiverCfg transceiverCfg)
{
  this->transceiverCfg = transceiverCfg;
  if(this->isPowered) {
    this->sendTransceiverCfg();
  }
}

// If the radio is on, command the radio to use the locally-stored config:
void Radio::sendTransceiverCfg()
{
  assert(this->isPowered);
  this->radioSerialTxLen = sprintf(this->radioSerialTxBuf, "%s=%0i,%3.4f,%3.4f,%4s,%i,%4s\r\n", 
    RADIO_CMD_SET_GROUP, 
    this->transceiverCfg.bandwidth, 
    this->transceiverCfg.txFreq, 
    this->transceiverCfg.rxFreq, 
    this->transceiverCfg.txSubtone, 
    this->transceiverCfg.squelch, 
    this->transceiverCfg.rxSubtone
  );
  this->sendCmd();
}


// Gets locally-stored transceiver config:
TransceiverCfg Radio::getTransceiverCfg() 
{
  return this->transceiverCfg;
}


// Enables or bypasses pre/de-emphasis filter, high-pass filter, and low-pass filter
// If radio is on, this commands the radio to use this config, 
// otherwise just stores the config for use during Radio::start().
void Radio::setFiltCfg(FiltCfg filtcfg)
{
  this->filtCfg = filtcfg;   
  if(this->isPowered){
    this->sendFiltCfg();
  }
}


// Gets locally-stored radio filters config:
FiltCfg Radio::getFiltCfg()
{
  return this->filtCfg;
}


// Enables or bypasses pre/de-emphasis filter, high-pass filter, and low-pass filter
void Radio::sendFiltCfg()
{
  assert(this->isPowered);
  this->radioSerialTxLen = sprintf(this->radioSerialTxBuf, "%s=%0i,%0i,%0i\r\n", RADIO_CMD_SET_FILT, 
    (this->filtCfg.enableEmph ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS),
    (this->filtCfg.enableHPF  ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS),
    (this->filtCfg.enableLPF  ? RADIO_FILT_ENABLE : RADIO_FILT_BYPASS)
  );
  this->sendCmd();
  //fixme: verify expected response and only save this cfg if acknowledged
}


// Set the radio to transmit mode:
void Radio::transmitMode()
{
  digitalWrite(this->interfaces.radioModePin, RADIO_TX_STATE);
  this->transceiverMode = RADIO_MODE_TX;
}

// Set the radio to receive mode
void Radio::receiveMode()
{
  digitalWrite(this->interfaces.radioModePin, RADIO_RX_STATE);
  this->transceiverMode = RADIO_MODE_RX;
}


RadioMode Radio::getTransceiverMode()
{
  return this->transceiverMode;
}


// Checks for signal received on specified frequency
void Radio::scanFreq(double rxf)
{
  assert(this->isPowered);
  this->radioSerialTxLen = sprintf(radioSerialTxBuf, "%s+%3.4f\r\n", RADIO_CMD_SCAN_FREQ, rxf);
  this->sendCmd();
  //fixme: parse response
}


// Sets volume of received audio
void Radio::setVolume(uint8_t vol)
{
  assert(this->isPowered);    
  this->radioSerialTxLen = sprintf(radioSerialTxBuf, "%s=%0i\r\n", RADIO_CMD_SET_VOL, vol);
  this->sendCmd();
  //fixme: verify expected response
}


// Opens or closes radio tail tone
void Radio::setTail(bool openTail)
{
  assert(this->isPowered);
  this->radioSerialTxLen = sprintf(radioSerialTxBuf, "%s=%0i\r\n", RADIO_CMD_SET_TAIL, openTail);
  this->sendCmd();
  //fixme: verify expected response
}


// Evaluates radio signal stength indication  
uint8_t Radio::getSignalStrength()
{
  assert(this->isPowered);
  unsigned int rssi = 0;
  this->radioSerialTxLen = sprintf(radioSerialTxBuf, "%s?\r\n", RADIO_CMD_GET_RSSI);
  this->sendCmd();
  //fixme: parse response
  if(radioSerialRxLen>0){
    sscanf(this->radioSerialRxBuf, "RSSI=%03u", &rssi);
  }
  return (uint8_t)rssi;
}


// Queries radio module firmware version
void Radio::getVer()
{
  assert(this->isPowered);
  this->radioSerialTxLen = sprintf(radioSerialTxBuf, "%s?\r\n", RADIO_CMD_GET_VER);
  this->sendCmd();
  //fixme: parse response
}


// Sets Rx frequency as specified, and Tx frequency with + or - 600 Hz offset 
void Radio::setRxFreqWithTxOffset(double rxFreq, RadioTxOffset offsetSign)
{
  double txOffset = (double)(offsetSign==RADIO_TX_OFFSET_PLUS ? +600 : -600);
  this->transceiverCfg.rxFreq = rxFreq;
  this->transceiverCfg.txFreq = rxFreq + txOffset;
  this->sendTransceiverCfg();
}


// Sets both Rx and Tx frequency to specified value:
void Radio::setFreq(double freq)
{
  this->transceiverCfg.rxFreq = freq;
  this->transceiverCfg.txFreq = freq;
  this->sendTransceiverCfg();
}


// Sets Tx and Rx frequencies separately:
void Radio::setTxAndRxFreq(double txFreq, double rxFreq)
{
  this->transceiverCfg.rxFreq = rxFreq;
  this->transceiverCfg.txFreq = txFreq;
  this->sendTransceiverCfg();
}


// Set squelch:
void Radio::setSquelch(uint8_t squelch)
{
  this->transceiverCfg.squelch = squelch;
  this->sendTransceiverCfg();
}


// Sends serial buffer radioSerialTxBuf to the radio
void Radio::sendCmd()
{
  assert(this->isPowered);
  #ifdef RADIO_DEBUG
    this->interfaces.LogSerial->print("To radio: ");
    this->interfaces.LogSerial->print(radioSerialTxBuf);
  #endif
  this->interfaces.RadioSerial->flush();
  // Send cmd to radio:
  this->interfaces.RadioSerial->print(this->radioSerialTxBuf);
  delay(1);
  // Receive response from radio cmd interface:
  this->receiveReply();
  #ifdef RADIO_DEBUG
    this->interfaces.LogSerial->print("From radio: ");
    this->interfaces.LogSerial->println(this->radioSerialRxBuf);
  #endif
}



// Waits for acknowledgement from SA818 command interface
void Radio::receiveReply()
{
  assert(this->isPowered);
  
  unsigned char readByte;
  
  // wait here until receive byte
  // fixme:  add timeout here
  while(!this->interfaces.RadioSerial->available()){
    delay(1);
  }

  this->radioSerialRxLen = 0;
    
  // Collect bytes until EOL or reach max buffer length:
  while(this->interfaces.RadioSerial->available()){

    // read one byte:
    readByte = this->interfaces.RadioSerial->read();
    
    // check if it's end of line:
    if(readByte=='\r' || readByte=='\n'){
      this->radioSerialRxBuf[this->radioSerialRxLen]='\0';
      return;
 
    // if receiving then receive:
    } else if(this->radioSerialRxLen<RADIO_SERIAL_RX_BUF_LEN) {
      this->radioSerialRxBuf[this->radioSerialRxLen++] = readByte;

    // if reached max allowable msg length then dump and reset buffer:
    } else if(radioSerialRxLen>=RADIO_SERIAL_RX_BUF_LEN) {
      this->interfaces.LogSerial->println("Error:  radio cmd reply exceeds buffer length.");
      this->radioSerialRxLen = 0;
      return;
    }
  }
}
