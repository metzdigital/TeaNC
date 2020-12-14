#include <teanc_core.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//Either remove this line and use the subsequent #defines instead,
//or write your own wifi_private.h file containing these #defines.
//#include "wifi_private.h"
const char wifi_ssid[] = "my_wifi_ssid";
const char wifi_password[] = "my_wifi_password";

// TeaNC core configuration:
TeancPeripheralsCfg peripheralsCfg = {
  enableGPS:  false,
  enableVHF:  true,
  enableLCD:  false,
};

WifiCfg wifiCfg = {
  mode:      WIFI_MODE_STA,  // WIFI_MODE_STA or WIFI_MODE_AP
  ssid:      wifi_ssid,
  password:  wifi_password,
};

WifiCfg fallbackWifiCfg = {
  mode:      WIFI_MODE_AP,  // WIFI_MODE_STA or WIFI_MODE_AP
  ssid:      (const char*)"teanc",
  password:  (const char*)"",
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
  enableEmph:   RADIO_FILT_ENABLE,
  enableHPF:    RADIO_FILT_ENABLE,
  enableLPF:    RADIO_FILT_ENABLE
};




// Audio sampling config:
const uint16_t adc_sample_freq = 32768;
const uint16_t i2s_in_buffer_len = 1024;

// Globals:
TaskHandle_t TaskHandle_2;
boolean streaming       = true;   // ADC enabler from web
const uint16_t i2s_in_dma_buffer_len = i2s_in_buffer_len;
const uint16_t ws_audio_buffer_len = i2s_in_buffer_len;
uint16_t* i2s_read_buff = (uint16_t*)calloc(i2s_in_buffer_len, sizeof(uint16_t));
uint16_t* ws_send_buffer = (uint16_t*)calloc(ws_audio_buffer_len, sizeof(uint16_t));
size_t bytes_read;

// Prototypes:
static const inline void Sampling();
static const inline void SendData();


// I2S Audio sampling configuration:
i2s_port_t i2s_in_port = I2S_NUM_0;   // I2S Port; I think this is always I2S_NUM_0.. maybe move this to teanc_core
adc1_channel_t adc_channel = ADC1_CHANNEL_0; //GPIO36 (see adc_channel.h in esp32 core)  this should always be _0.. maybe move to teanc_core 

void configure_i2s(){
  i2s_config_t i2s_in_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),  // I2S receive mode with ADC
    .sample_rate = adc_sample_freq,                                               // set I2S ADC sample rate
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,                                 // 16 bit I2S (even though ADC is 12 bit)
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,                                 // handle adc data as single channel (right)
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S,               // I2S format
    .intr_alloc_flags = 0,                                                        // 
    .dma_buf_count = 32,                                                           // number of DMA buffers >=2 for fastness
    .dma_buf_len = i2s_in_dma_buffer_len,                                                // number of samples per buffer
    .use_apll = false,                                                            // no Audio PLL - buggy and not well documented
//    .tx_desc_auto_clear = true
  };
  adc1_config_channel_atten(adc_channel, ADC_ATTEN_11db);
  adc1_config_width(ADC_WIDTH_12Bit);
  ESP_ERROR_CHECK( i2s_set_adc_mode(ADC_UNIT_1, adc_channel) );
  i2s_driver_install(i2s_in_port, &i2s_in_config, 0, NULL);
  i2s_adc_enable(i2s_in_port);
}


// Web server and websocket:
AsyncWebServer    server(80);
AsyncEventSource  events("/events");
AsyncWebServerRequest *request;
AsyncWebSocket    ws("/ws");

// Websocket event handler:
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      SerialUSB->printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      SerialUSB->printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      SerialUSB->print("received instruction over websocket: ");
      SerialUSB->println((char*)data);
      //handleWebSocketMessage(arg, data, len);
      //break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      SerialUSB->println("Unhandled websocket event");
      break;
  }
}

// Saving this for later.. parse command received over websocket:
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      // do something in response to "toggle" cmd for example
    }
  }
}

// HTTP port 80 server-ons:
void serverons(){
  
  // Start sampling and websocket streaming:
  server.on("/Start", HTTP_GET, [](AsyncWebServerRequest * request) {
    SerialUSB->println("starting stream");
    request->send(204);
    streaming = true;
  });

  // Stop sampling and websocket streaming:
  server.on("/Stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    SerialUSB->println("stopping stream");
    request->send(204);
    streaming = false;
  });

  // Get received signal strength:
  server.on("/getRssi", HTTP_GET, [](AsyncWebServerRequest * request) {
    String message = (String)VHF.getSignalStrength();
    //SerialUSB->print("reporting rssi: " + message);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", message);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Set various radio parameters:
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    //List all parameters (Compatibility)
    int args = request->args();
    
    String message = "Number of args received:";
    message += args;            //Get number of parameters
    message += "\n";                            //Add a new line
    for (int i=0; i<args; i++) {
      message += "Arg " + (String)i + ": ";   //Include the current iteration value
      message += request->argName(i) + ": ";     //Get the name of the parameter
      message += request->arg(i) + "\n";              //Get the value of the parameter
      SerialUSB->println(message);
    } 

    //Check if GET parameter exists
    if(request->hasParam("freq")){
      AsyncWebParameter* p = request->getParam("freq");
      float freq = p->value().toFloat();
      if(freq>=RADIO_FREQ_MHZ_MIN && freq<=RADIO_FREQ_MHZ_MAX){
        message += "setting frequency to " + (String)freq + "Hz \n";
        VHF.setFreq(freq);
      } else {
        message += "requested frequency " + (String)p->value().c_str() + " is out of range.\n";
      }
    }
    
    request->send(200, "text/plain", message);       //Response to the HTTP request
  });

}



void setup() {
  SerialUSB->begin(250000);
  SerialUSB->println("VHF rx -> websockets test");
  
  digitalWrite(USER_LED_PIN, HIGH);

  teanc.begin();


  //VHF RADIO SETUP:
  SerialUSB->println("Turning on VHF radio");
  VHF.powerOn();
  
  VHF.getVer();

  //ADC Setup:
  configure_i2s();

  // Start web server:
  server.begin();
  serverons();

  // Start websocket:
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Create audio sampling task:
  xTaskCreatePinnedToCore(samplingLoop, "v_getData", 80000, NULL, 0, &TaskHandle_2, 0 );
}




void loop() {
  vTaskDelay(1);

  if(digitalRead(USER_PUSH_BTN1_PIN) == HIGH){
    digitalWrite(USER_LED_PIN, HIGH);
  }
  else if(digitalRead(USER_PUSH_BTN2_PIN) == HIGH){
    digitalWrite(USER_LED_PIN, HIGH);
    ESP.restart();
  }
  else{
    digitalWrite(USER_LED_PIN, LOW);
  }
}


static void samplingLoop(void * pvParameters){
  for( ;; ){
    vTaskDelay(1);  // REQUIRED TO RESET THE WATCH DOG TIMER IF WORKFLOW DOES NOT CONTAIN ANY OTHER DELAY
    Sampling();
  }
}


static const inline void Sampling(){
  i2s_read(i2s_in_port, (void*)i2s_read_buff, i2s_in_buffer_len*sizeof(uint16_t), &bytes_read, portMAX_DELAY);
  if(streaming && I2S_EVENT_RX_DONE && bytes_read>0){
    SendData();
  }
}


static const inline void SendData(){
  //String data;
  //SerialUSB->println("sending adc data");

  int16_t adc_data[ws_audio_buffer_len];

  //// Per esp32.com forum topic 11023, esp32 swaps even/odd samples,
  ////   i.e. samples 0 1 2 3 4 5 are stored as 1 0 3 2 5 4 ..
  ////   Have to deinterleave manually; use xor "^1" to leap frog indices
  //// Also need to mask upper 4 bits which contain channel info (see gitter chat between me-no-dev and bzeeman)  
  for(int i=0; i<ws_audio_buffer_len; i++){  // caution: this is not robust to odd buffer lens
    adc_data[i^1]   = (int16_t)(i2s_read_buff[i] & 0x0FFF);
  }
  memcpy(ws_send_buffer, adc_data, ws_audio_buffer_len*sizeof(uint16_t));

  // Broadcast binary data to websocket:
  ws.binaryAll((uint8_t *)&ws_send_buffer[0], ws_audio_buffer_len*sizeof(uint16_t)); 
}
