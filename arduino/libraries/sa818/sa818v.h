#ifndef sa818v_h
#define sa818v_h

#define RADIO_DEBUG

#define RADIO_SERIAL_BAUD 9600
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

static char RADIO_CTCS_DISABLE[] = "0000";


#define RADIO_TAIL_OPEN  1
#define RADIO_TAIL_CLOSE 0

#define RADIO_FILT_BYPASS 1
#define RADIO_FILT_ENABLE 0

#define RADIO_SQUELCH_OPEN 0

#define RADIO_TX_STATE 	LOW
#define RADIO_RX_STATE 	HIGH

#define RADIO_PWR_ON_STATE 	HIGH
#define RADIO_PWR_OFF_STATE LOW


typedef enum {
  RADIO_TX_OFFSET_PLUS,
  RADIO_TX_OFFSET_MINUS
} RadioTxOffset;


typedef enum {
  RADIO_MODE_RX,
  RADIO_MODE_TX,
} RadioMode;

typedef struct RadioInterfaces {
  uint8_t powerEnablePin;
  uint8_t radioModePin;
  Stream* RadioSerial;
  Stream* LogSerial;
} RadioInterfaces;

typedef struct TransceiverCfg {
  uint8_t bandwidth;
  double txFreq;
  double rxFreq;
  char* txSubtone;
  char* rxSubtone;
  uint8_t squelch;
} TransceiverCfg;

typedef struct FiltCfg {
  bool enableEmph;
  bool enableHPF;
  bool enableLPF;
} FiltCfg;


class Radio {
  public: 
    Radio();
    Radio(RadioInterfaces interfaces);
    ~Radio();
    
    void setup(RadioInterfaces interfaces);
    void setup(RadioInterfaces interfaces, TransceiverCfg transceiverCfg);
    void setup(RadioInterfaces interfaces, TransceiverCfg transceiverCfg, FiltCfg filtcfg);
    void setup(TransceiverCfg transceiverCfg);
    void setup(TransceiverCfg transceiverCfg, FiltCfg filtcfg);

    
    void powerOn();    
    void powerOff();

    void setTransceiverCfg(TransceiverCfg transceiverCfg);
    TransceiverCfg getTransceiverCfg();
    
//    void setLogSerial(Stream *streamObj) { LogSerial = streamObj; logSerialAttached=true; }  
        
    void transmitMode();
    void receiveMode();    
    RadioMode getTransceiverMode();
        
    void setFiltCfg(FiltCfg filtcfg);
    FiltCfg getFiltCfg();
 
    // Basic radio command wrappers:
    void scanFreq(double rxf);
    void setVolume(uint8_t vol);
    void setTail(bool openTail);
    uint8_t getSignalStrength();
    void getVer();
    
    // Additional helpers:
    void setRxFreqWithTxOffset(double rxFreq, RadioTxOffset offsetSign); 
    void setFreq(double freq);
    void setTxAndRxFreq(double txFreq, double rxFreq);
    void setSquelch(uint8_t squelch);
  
  private:  
    RadioInterfaces interfaces;  
    bool interfacesDefined = false;
    
    TransceiverCfg transceiverCfg = {
      bandwidth:  RADIO_BANDWIDTH_12500_HZ,
      txFreq:     144.600,
      rxFreq:     144.600,
      txSubtone:  RADIO_CTCS_DISABLE,
      rxSubtone:  RADIO_CTCS_DISABLE,
      squelch:    0,
    };
        
    FiltCfg filtCfg = {
      enableEmph: true,
      enableHPF:  true,
      enableLPF:  true
    };

    bool logSerialAttached = false;    
    
    void sendTransceiverCfg();
    void sendFiltCfg();
    

    char radioSerialTxBuf[RADIO_SERIAL_TX_BUF_LEN] = {0};
    char radioSerialRxBuf[RADIO_SERIAL_RX_BUF_LEN] = {0};
    uint8_t radioSerialTxLen = 0;
    uint8_t radioSerialRxLen = 0;

    bool isPowered = false;
    
    RadioMode transceiverMode;

    void sendCmd();
    void receiveReply();
};




#endif
