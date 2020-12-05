#include <Arduino.h>
#include <WiFi.h>
//#include <WebSocketsServer.h>
#include <driver/i2s.h>
#include <WebAuthentication.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define FS_NO_GLOBALS
#include <FS.h>
#include "SPIFFS.h"
#include <driver/i2s.h>
#include <driver/adc.h>

#include <teanc_core.h>
#include <sa818v.h>

//SSID and Password of your WiFi router
#include "my_wifi.h"  // this contains my ssid and pw; remove this line or make your own
const char* ssid = MY_WIFI_SSID;
const char* password = MY_WIFI_PASSWORD;


/////// TEANC SETUP ////////////////////
Radio vhf;

/////////////////////////////////////////////


AsyncWebServer    server(80);
//WebSocketsServer  webSocket = WebSocketsServer(81);
AsyncEventSource  events("/events");
AsyncWebServerRequest *request;
AsyncWebSocket    ws("/ws");

/** CONFIG **/
i2s_port_t i2s_in_port = I2S_NUM_0;   // I2S Port 
adc1_channel_t adc_channel = ADC1_CHANNEL_0; //GPIO36  (SET THIS FOR YOUR HARDWARE) (see adc_channel.h)
const uint16_t adc_sample_freq = 32768;
const uint16_t i2s_in_buffer_len = 1024;
const uint16_t i2s_in_dma_buffer_len = i2s_in_buffer_len;
const uint16_t ws_audio_buffer_len = i2s_in_buffer_len;

/** GLOBALS **/
TaskHandle_t TaskHandle_2;
boolean streaming       = true;   // ADC enabler from web

uint16_t* i2s_read_buff = (uint16_t*)calloc(i2s_in_buffer_len, sizeof(uint16_t));
uint16_t* ws_send_buffer = (uint16_t*)calloc(ws_audio_buffer_len, sizeof(uint16_t));
size_t bytes_read;

AsyncWebSocketClient * globalClient = NULL;


// Prototypes:

//void configure_i2s();
//static const inline void SendData();
static const inline void Sampling();




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

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
}




void serverons(){

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    while(!SPIFFS.exists("/index.html")){
      Serial.println("Error, /index.html is not onboard");
      delay(1000);
    }
    Serial.println("trying to send index.html");
    AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    response->addHeader("Access-Control-Max-Age", "10000");
    response->addHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    request->send(response);
    Serial.println("sent");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Sending /favicon.ico");
    request->send(SPIFFS, "/favicon.ico","text/css");
  });

  server.on("/jqueryjs", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("sending /jquery.js.gz");
    AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/jquery.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/CanvasJs", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("Sending /CanvasJs.js.gz");
    AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/CanvasJs.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  server.on("/Stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("stopping stream");
    request->send(204);
    streaming = false;
  });
  
  server.on("/Start", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("starting stream");
    request->send(204);
    streaming = true;
  });

  server.on("/getRssi", HTTP_GET, [](AsyncWebServerRequest * request) {
    String message = (String)vhf.getSignalStrength();
    //Serial.print("reporting rssi: " + message);
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", message);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

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
      Serial.println(message);
    } 

    //Check if GET parameter exists
    if(request->hasParam("freq")){
      AsyncWebParameter* p = request->getParam("freq");
      float freq = p->value().toFloat();
      if(freq>=RADIO_FREQ_MHZ_MIN && freq<=RADIO_FREQ_MHZ_MAX){
        message += "setting frequency to " + (String)freq + "Hz \n";
        RadioCfg cfg = vhf.radioCfg;
        cfg.txf = freq;
        cfg.rxf = freq;
        vhf.setConfig(cfg);
      } else {
        message += "requested frequency " + (String)p->value().c_str() + " is out of range.\n";
      }
    }
    
    request->send(200, "text/plain", message);       //Response to the HTTP request
  });

}



void setup() {
  Serial.begin(250000);
  Serial.println("VHF rx -> websockets test");
  
  digitalWrite(USER_LED_PIN, HIGH);


  //WIFI_Setup();
  WiFi.begin(ssid, password);     //Connect to your WiFi network
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Wifi connected. IP: ");
  Serial.println(WiFi.localIP());


  //VHF RADIO SETUP:
  vhf.setLogSerial(&Serial);  // could send this to telnet instead!
  Serial.println("Initializing radio..");
  vhf.init();
  Serial.println("getting radio module version");
  vhf.getVer();
  Serial.println("configuring radio..");
  struct RadioCfg cfg = {
    bandwidth:    RADIO_BANDWIDTH_12500_HZ,
    txf:          145.000,
    rxf:          145.000,
    tx_subaudio:  RADIO_CTCS_DISABLE,
    squelch:      0,
    rx_subaudio:  RADIO_CTCS_DISABLE
  };
  vhf.setConfig(cfg);
  
  //Serial.println("disabling filters");
  struct FiltCfg fcfg = {
    enableEmph:   RADIO_FILT_BYPASS,
    enableHPF:    RADIO_FILT_BYPASS,
    enableLPF:    RADIO_FILT_BYPASS
  };
  vhf.setFilter(fcfg);


  //ADC Setup:
  configure_i2s();
  SPIFFS.begin() ? Serial.println("SPIFFS.OK") : Serial.println("SPIFFS.FAIL");

  server.begin();
  serverons();

  initWebSocket();
//  webSocket.begin();
  xTaskCreatePinnedToCore ( samplingLoop, "v_getData", 80000, NULL, 0, &TaskHandle_2, 0 );
}

void loop() {
  //webSocket.loop();       // WEBSOCKET PACKET LOOP
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
    vTaskDelay(1);          // REQUIRED TO RESET THE WATCH DOG TIMER IF WORKFLOW DOES NOT CONTAIN ANY OTHER DELAY
    Sampling();
  }
}

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






static const inline void SendData(){
  //String data;
  //Serial.println("sending adc data");

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
//  webSocket.sendBIN(0, (uint8_t *)&ws_send_buffer[0], ws_audio_buffer_len*sizeof(uint16_t)); 
}



static const inline void Sampling(){
  i2s_read(i2s_in_port, (void*)i2s_read_buff, i2s_in_buffer_len*sizeof(uint16_t), &bytes_read, portMAX_DELAY);
  if(streaming && I2S_EVENT_RX_DONE && bytes_read>0){
    SendData();
  }
}
