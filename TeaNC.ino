

#define USER_LED_PIN 5
#define USER_PUSH_BTN1_PIN 39
#define USER_PUSH_BTN2_PIN 34

#define POWER_SENSE_PIN 4

#define DISPLAY_ENABLE_PIN 19
#define DISPLAY_I2C_SCL_PIN 22
#define DISPLAY_I2C_SDA_PIN 21

#define GPS_UART_RX_PIN 27
#define GPS_UART_TX_PIN 26
#define GPS_PPS_PIN 35

#define VHF_UART_RX_PIN 33
#define VHF_UART_TX_PIN 32
#define VHF_PWR_DOWN_PIN 13
#define VHF_RX_TXN_PIN 14
#define VHF_AOUT_PIN 25
#define VHF_AIN_PIN 36

#define VHF_TX_STATE LOW
#define VHF_RX_STATE HIGH
#define VHF_PWR_ON  HIGH
#define VHF_PWR_OFF  LOW

#define CHAR_LF 0x0A

HardwareSerial *SerialGPS;

void setup() {
  //Setup the GPS pin modes
  pinMode(GPS_UART_TX_PIN, OUTPUT);
  pinMode(GPS_UART_RX_PIN, INPUT);
  pinMode(GPS_PPS_PIN, INPUT);

  //Setup the VHF pin modes
  pinMode(VHF_UART_TX_PIN, OUTPUT);
  pinMode(VHF_UART_RX_PIN, INPUT);
  pinMode(VHF_RX_TXN_PIN, OUTPUT);
  pinMode(VHF_PWR_DOWN_PIN, OUTPUT);

  //User push buttons and LED
  pinMode(USER_LED_PIN, OUTPUT);
  pinMode(USER_PUSH_BTN1_PIN, INPUT);
  pinMode(USER_PUSH_BTN2_PIN, INPUT);

  //Display pin modes
  pinMode(DISPLAY_ENABLE_PIN, OUTPUT);

    
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, GPS_UART_RX_PIN, GPS_UART_TX_PIN);
  Serial2.begin(9600, SERIAL_8N1, VHF_UART_RX_PIN, VHF_UART_TX_PIN);

  Serial.println("ESP32 GPS testing");
  digitalWrite(USER_LED_PIN, HIGH);
  digitalWrite(VHF_PWR_DOWN_PIN, VHF_PWR_ON);
  digitalWrite(VHF_RX_TXN_PIN, VHF_RX_STATE);
  delay(1000);

  
  Serial2.println("AT+DMOCONNECT");
  char ret = '\0';
  while(ret != CHAR_LF){
    if(Serial2.available()){
      ret = Serial2.read();
      Serial.write(ret);
    }
  }
  Serial2.println("AT+DMOSETGROUP=1,145.0000,145.0000,0000,1,0000");

}

void loop() {
  char temp;
  uint8_t bytes; 
  static uint8_t val=0;

  
  while(Serial1.available()){
        temp = Serial1.read();
        Serial.print(temp);
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
  
  while(Serial2.available()){
      temp = Serial2.read();
      Serial.write(temp);
  }
  /*
  for(int i=0;i<256;i++){
    dacWrite(VHF_AOUT_PIN,i);
    delayMicroseconds(10);
  }
  for(int i=254;i>0;i--){
    dacWrite(VHF_AOUT_PIN,i);
    delayMicroseconds(10);
  }
  */


}
