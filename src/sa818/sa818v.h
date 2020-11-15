#ifndef sa818v_h
#define sa818v_h

#define RADIO_DEBUG

#define RADIO_SERIAL_RX_BUF_LEN 60
#define RADIO_SERIAL_TX_BUF_LEN 30
#define RADIO_SERIAL_EOL '\r'

#define RADIO_CMD_CONNECT   "AT+DMOCONNECT"
#define RADIO_CMD_SET_GROUP "AT+DMOSETGROUP"
#define RADIO_CMD_SCAN_FREQ "S+"
#define RADIO_CMD_SET_VOL   "AT+DMOSETVOLUME"
#define RADIO_CMD_SET_FILT  "AT+SETFILTER"
#define RADIO_CMD_SET_TAIL  "AT+SETTAIL"
#define RADIO_CMD_GET_RSSI  "RSSI"
#define RADIO_CMD_GET_VER   "AT+VERSION"

#define RADIO_BANDWIDTH_12500_HZ 0
#define RADIO_BANDWIDTH_25000_HZ 1

#define RADIO_FREQ_MHZ_MIN 134.000
#define RADIO_FREQ_MHZ_MAX 174.000

#define RADIO_CTCS_DISABLE "0000"

#define RADIO_TAIL_OPEN  1
#define RADIO_TAIL_CLOSE 0

#define RADIO_FILT_BYPASS 1
#define RADIO_FILT_ENABLE 0

#define RADIO_SQUELCH_OPEN 0

#define RADIO_TX_STATE 	LOW
#define RADIO_RX_STATE 	HIGH

#define RADIO_PWR_ON_STATE 	HIGH
#define RADIO_PWR_OFF_STATE LOW


struct RadioCfg{
  uint8_t bandwidth;
  double txf;
  double rxf;
  char* tx_subaudio;
  uint8_t squelch;
  char* rx_subaudio;
};

struct FiltCfg{
  bool enableEmph;
  bool enableHPF;
  bool enableLPF;
};


class Radio {
  public: 
    Radio(HardwareSerial *streamObj);
    ~Radio();
    
    void setControlSerial(HardwareSerial *streamObj) { ControlSerial = streamObj; controlSerialAttached=true; }
    void setLogSerial(HardwareSerial *streamObj) { LogSerial = streamObj; logSerialAttached=true; }  
    
    void connect();
	void transmitMode();
	void receiveMode();
	void powerOff();
	void powerOn();
    void setConfig(uint8_t bandwidth, double txf, double rxf, char* tx_subaudio, uint8_t squelch, char* rx_subaudio);
    void setConfig(RadioCfg cfg);
    void scanFreq(double rxf);
    void setVolume(uint8_t vol);
    void setFilter(bool enableEmph, bool enableHPF, bool enableLPF);
    void setFilter(FiltCfg cfg);
    void setTail(bool openTail);
    uint8_t getSignalStrength();
    void getVer();

    RadioCfg radioCfg;

    FiltCfg filtCfg;
  
  private:  
    HardwareSerial *ControlSerial;
    HardwareSerial *LogSerial;
    char serialTxBuf[RADIO_SERIAL_TX_BUF_LEN] = {0};
    char serialRxBuf[RADIO_SERIAL_RX_BUF_LEN] = {0};
    uint8_t serialTxLen = 0;
    uint8_t serialRxLen = 0;
    bool controlSerialAttached = false;
    bool logSerialAttached = false;
	bool isPowered = false;
    void sendCmd();
    void receiveReply();
};




#endif
