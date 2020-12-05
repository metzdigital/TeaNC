#ifndef TEANC_CORE_H_
#define TEANC_CORE_H_


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


RetStatus_t configIO(void);

#endif 