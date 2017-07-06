#ifndef CM530_P04_API_h
#define CM530_P04_API_h
#define CM530_FIRMWARE_VERSION          "v2012.02.15.1600\n" // Sucky version >_>
#define USING_DYNAMIXEL
#define USING_PC_UART
	//#define USING_ZIGBEE

#ifndef DEBUGMODE
#define DEBUGMODE
#endif
#include "stm32f10x_it.h"
#include "Definitions.h"
#include <stdbool.h>

#define START_PROGRAM int main(void) {\
	SysInit();

#define END_PROGRAM		end();\
	return 0;\
}

/***********************************************************************************************************************/

#ifdef USING_DYNAMIXEL
extern uint32 Baudrate_DXL;
#endif
#ifdef USING_ZIGBEE
extern uint32 Baudrate_ZIG;
#endif
#ifdef USING_PC_UART
extern uint32 Baudrate_PCU;
#endif

extern EasyPort_t EasyButton[6], EasyLED[7], EasyEPort[12];

// Structure to represent the robot's physical servo positions.
typedef struct {
	struct {
		struct {
			unsigned char shoulder;
			unsigned char elbow;
			unsigned char hand;
		} arm;
		struct {
			unsigned char hip;
			unsigned char knee;
			unsigned char foot;
		} leg;
	} left;
	struct {
		struct {
			unsigned char shoulder;
			unsigned char elbow;
			unsigned char hand;
		} arm;
		struct {
			unsigned char hip;
			unsigned char knee;
			unsigned char foot;
		} leg;
	} right;
} robot;				// Redefine a struct to represent the robot, whom is named Gizmo.

extern uint8						DXL_STATUS;

extern robot						Gizmo;
extern bool							TorqueState;
extern volatile uint8		gbPacketWritePointer;
extern volatile uint8		gbPacketReadPointer;
extern volatile uint8		gbpPacketDataBuffer[16+1+16];

	// Will append a newline to the string, then print like normal
void printLine(const char* string);

	// Takes the given string and sends it to the RoboPlus terminal
void printString(const char* string);

void printID(uint8 id);
/**
 *	Function to properly end the program.
 *	@return void does not return anything.
 **/
void end(void);

// Gizmo functions
void gizmo_init(void);
void setGizmoLeftArm(uint8 shoulder, uint8 elbow, uint8 hand);
void setGizmoLeftLeg(uint8 hip, uint8 knee, uint8 foot);
void setGizmoRightArm(uint8 shoulder, uint8 elbow, uint8 hand);
void setGizmoRightLeg(uint8 hip, uint8 knee, uint8 foot);

// Debugger functions

// LED functions
void led_init(void);
void flashLEDs(uint8* LEDS);
void blinkLEDs(void);
void blinkMultiLEDs(uint8* LEDs);
void setLED(LED_t led, uint8 state);
void setLEDsOff(void);
void setLEDsOn(void);
void setLEDs(bool state);

void dxl_init(void);
void verbose_ping_allDXL(void);
void verbose_ping_dxl(uint8 id);
void ping_dxl(uint8 id);
void dxl_check(void);
void printStatus(void);
// Dynamixel functions

void printAllServoInfo(void);
void printServoInfo(uint8 id);

/*******************************************************
 *	@author Sam Watkins
 *	@date 16 May 2013
 *	@functiongroup Dynamixel API
 *	@discussion A set of useful functions for reading and writing
 *		to the Dynamixel servos used with the Bioloid Premium.
*******************************************************/

/**
 *	Function to get the current firmware version
 *	@param id The ID of  a specific DXL Servo
 *	@return Returns the firmware number
 **/
uint8  getDXLFirmware(uint8 id);
/**
 *	Function to get the used baudrate
 *	@param id The ID of  a specific DXL Servo
 *	@return Returns the baudrate
 **/
uint8  getDXLBaudrate(uint8 id);
/**
 *	Function to get the status level of a DXL
 *	@param id The ID of  a specific DXL Servo
 *	@return Returns the status level
 **/
uint8  getDXLStatus(uint8 id);
/**
 *	Function to get the state of Torque
 *	@param id The ID of a specific DXL Servo
 *	@return Returns 1 if Torque is on, or 0 if it is off.
 **/
uint8  getDXLTorqueState(uint8 id);
/**
 *	Function to determine if the DXL is in motion or not.
 *	@param id The ID of a specific DXL Servo
 *	@return Returns 1 if it's in motion, or 0 if it's not moving.
 **/
uint8  getDXLCurrentState(uint8 id);
/**
 *	Function to check the current voltage of a servo
 *	@param id The ID of a specific DXL Servo
 *	@return Returns the volt level.
 **/
uint8	 getDXLCurrentVoltage(uint8 id);
/**
 *	Function to check the temperature of a servo
 *	@param id The ID of a specific DXL Servo
 *	@return Returns the current temperature
 **/
uint8	 getDXLCurrentTemperature(uint8 id);
/**
 *	Function to get the model number of a DXL
 *	@param id The ID of  a specific DXL Servo
 *	@return Returns the model number
 **/
uint16 getDXLModel(uint8 id);
/**
 *	Function to read the set goal position
 *	@param id The ID of a specific DXL Servo
 *	@return Returns the goal position of the DXL
 **/
uint16 getDXLGoalPosition(uint8 id);
/**
 *	Function to check for a DXL's current position
 *	@param id The ID of a specific DXL Servo
 *	@return Returns the integer value of it's current position
 **/
uint16 getDXLCurrentPosition(uint8 id);
/**
 *	Function to get the DXL's speed.
 *	@param id The ID of a specific DXL Servo
 *	@return Returns the DXLs current speed.
 **/
uint16 getDXLCurrentSpeed(uint8 id);
/**
 *	Function to determine a DXL's current Load. (Not sure what 'Load' is in DXL terms).
 *	@param id The ID of a specific DXL Servo
 *	@return Returns the current load.
 **/
uint16 getDXLCurrentLoad(uint8 id);

void setDXLParams(void);


void setDXLPosition(uint8 id, int pos);


void setDXLTorqueState(uint8 id, bool state);


void setDXLLEDState(uint8 id, bool state);


void setTorqueOff(void);

void setTorqueOn(void);

void setTorque(bool state);

void switchTorque(void);



	// USART functions
void USART_Configuration(uint8 PORT, uint32 baudrate);

void DisableUSART1(void);




// CM-530 Helper functions - NaN
void SetLED(LED_t led, uint8 state);
uint8 ReadButton(Button_t button);
void Buzzed(uint32 mlength, uint32 tone);
void PlayNote(uint32 mlength, buzzed_note_t note, uint8 octave);
void SetEPort(EPortD_t pin, uint8 state);
uint16 ReadAnalog(EPortA_t port);
uint16 ReadIR(EPortA_t port);





uint8 dxl_hal_open(uint32 baudrate);
void dxl_hal_close(void);
void dxl_hal_clear(void);
uint8 dxl_hal_tx(uint8* pPacket, uint8 numPacket);
uint8 dxl_hal_rx(uint8* pPacket, uint8 numPacket);
void dxl_hal_set_timeout(uint8 NumRcvByte);
uint8 dxl_hal_timeout(void);
void RxD_DXL_Interrupt(void);

// Dynamixel SDK platform independent header
uint8 dxl_initialize(uint8 baudrate);
void dxl_terminate(void);
void dxl_tx_packet(void);
void dxl_rx_packet(void);
void dxl_clear_statpkt(void);
void dxl_set_txpacket_id(uint8 id);
void dxl_set_txpacket_instruction(uint8 instruction);
void dxl_set_txpacket_parameter(uint8 index, uint8 value);
void dxl_set_txpacket_length(uint8 length);
uint8 dxl_get_rxpacket_error(uint8 errbit);
uint8 dxl_get_rxpacket_parameter(uint8 index);
uint8 dxl_get_rxpacket_length(void);
uint16 dxl_makeword(uint8 lowbyte, uint8 highbyte);
uint8 dxl_get_lowbyte(uint16 word);
uint8 dxl_get_highbyte(uint16 word);
void dxl_txrx_packet(void);
uint16 dxl_get_result(void);
uint16 dxl_ping(uint8 id);
uint8 dxl_read_byte(uint8 id, uint8 address);
int dxl_write_byte(uint8 id, uint8 address, uint8 value);
uint16 dxl_read_word(uint8 id, uint8 address);
void dxl_write_word(uint8 id, uint8 address, uint16 value);
void dxl_capture(uint8 id);



#ifdef USING_PC_UART

// Serial/PC_UART platform independent header
uint8 pcu_initialize(uint32 baudrate);
void pcu_terminate(void);
uint8 pcu_get_qstate(void);
// stdio.h compatibility
int std_putchar(char);
int std_puts(const char*);
int std_getchar(void);
char* std_gets(char*);
void PrintCommStatus(uint16);
void PrintErrorCode(void);
int PrintChar(char c);
int PrintLine(const char* string);
int PrintString(const char* s);
int GetChar(void);
char* GetString(char* s);
void Printu32d(uint32);
void Prints32d(int32);
void Printu16h(uint16);
void Printu8h(uint8);
#endif



#ifdef USING_ZIGBEE
// Zigbee SDK platform independent header

// RC-100 Button values
#define RC100_BTN_U                     0x0001
#define RC100_BTN_D                     0x0002
#define RC100_BTN_L                     0x0004
#define RC100_BTN_R                     0x0008
#define RC100_BTN_1                     0x0010
#define RC100_BTN_2                     0x0020
#define RC100_BTN_3                     0x0040
#define RC100_BTN_4                     0x0080
#define RC100_BTN_5                     0x0100
#define RC100_BTN_6                     0x0200


uint8 zgb_initialize(uint32 baudrate);
void zgb_terminate(void);
uint8 zgb_tx_data(uint16 data);
uint8 zgb_rx_check(void);
uint16 zgb_rx_data(void);

#endif




/**********************************************************************************************************************/

// CM-530 specific configuration/utility functions



void SysInit(void);
void StartCountdown(uint32);

extern volatile uint32 glCountdownCounter;

void uDelay(uint32);    // Actually uses a 10 [us] delay interval
void mDelay(uint32);    // Correctly uses 1 [ms] delay interval

//#define BATTERY_MONITOR_INTERVAL        10    // Check once every 10 seconds
//void start_timeout_bat(uint32);
#define VBUS_LOW_LIMIT                  115    // 11.5 Volts

void Battery_Monitor_Alarm(void);



#endif