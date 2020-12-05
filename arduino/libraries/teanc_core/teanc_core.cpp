
#include "Arduino.h"
#include "teanc_core.h"

RetStatus_t configIO(void){
	//Setup the GPS pin modes
	pinMode(GPS_UART_TX_PIN, OUTPUT);
	pinMode(GPS_UART_RX_PIN, INPUT);
	pinMode(GPS_PPS_PIN, INPUT);

	//User push buttons and LED
	pinMode(USER_LED_PIN, OUTPUT);
	pinMode(USER_PUSH_BTN1_PIN, INPUT);
	pinMode(USER_PUSH_BTN2_PIN, INPUT);

	//Display pin modes
	pinMode(DISPLAY_ENABLE_PIN, OUTPUT);
	
	return SUCCESS;
}