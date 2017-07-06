#include "API.h"

#ifdef USING_DYNAMIXEL
uint32 Baudrate_DXL = 1000000;
#endif
#ifdef USING_ZIGBEE
uint32 Baudrate_ZIG = 57600;
#endif
#ifdef USING_PC_UART
uint32 Baudrate_PCU = 57600;
#endif

#define USING_SYSTICK_10US
#define DEBUG_PRINT_VOLTAGE

EasyPort_t		// Tician's Easy-Components
EasyButton[6] = {
	{PORT_SW_UP, PIN_SW_UP},
	{PORT_SW_DOWN, PIN_SW_DOWN},
	{PORT_SW_LEFT, PIN_SW_LEFT},
	{PORT_SW_RIGHT, PIN_SW_RIGHT},
	{PORT_SW_START, PIN_SW_START},
	{PORT_MIC, PIN_MIC}
},
EasyLED[7] = {
	{GPIOC, (uint16)0x2000},
	{GPIOB, (uint16)0x2000},
	{GPIOB, (uint16)0x4000},
	{GPIOB, (uint16)0x8000},
	{GPIOC, (uint16)0x4000},
	{GPIOC, (uint16)0x8000},
	{GPIOB, (uint16)0x1000}
},
EasyEPort[12] = {
	{PORT_SIG_MOT1P, PIN_SIG_MOT1P},
	{PORT_SIG_MOT1M, PIN_SIG_MOT1M},
	{PORT_SIG_MOT2P, PIN_SIG_MOT2P},
	{PORT_SIG_MOT2M, PIN_SIG_MOT2M},
	{PORT_SIG_MOT3P, PIN_SIG_MOT3P},
	{PORT_SIG_MOT3M, PIN_SIG_MOT3M},
	{PORT_SIG_MOT4P, PIN_SIG_MOT4P},
	{PORT_SIG_MOT4M, PIN_SIG_MOT4M},
	{PORT_SIG_MOT5P, PIN_SIG_MOT5P},
	{PORT_SIG_MOT5M, PIN_SIG_MOT5M},
	{PORT_SIG_MOT6P, PIN_SIG_MOT6P},
	{PORT_SIG_MOT6M, PIN_SIG_MOT6M}
};
const uint8 AX12Servos[26]	=	{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0};
const uint8 AX12_IDS[18]		=	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
const uint8 NUM_AX12_SERVOS =	sizeof(AX12_IDS)/sizeof(uint8);

uint8							DXL_STATUS;

robot							Gizmo;							// Boom that just happened.
bool							TorqueState = false;
volatile uint8		gbPacketWritePointer;
volatile uint8		gbPacketReadPointer;
volatile uint8		gbpPacketDataBuffer[16+1+16];


/******************************		Private functions		******************************/
void printStatus(void);
void _delay_ms(int time);	// Legacy-support for CM510 libs.
int htoi(char digit);			// Hexadecimal value to Integer value conversion function.
void pcu_put_byte(uint8 bTxdData);
char readSerialChar();
char* readSerialLine();

	// Serial and Debug functions
char readSerialChar() {
		// Will *attempt* to read a character, or if 10 seconds passes and nothing happens then it returns nothing.
	char c;
	uint16 timeout = 1000;
	while (timeout > 0) {
		if (gbPacketReadPointer != gbPacketWritePointer) break;
		mDelay(1);
		timeout--;

		c = gbpPacketDataBuffer[gbPacketReadPointer];
		gbPacketReadPointer++;
		gbPacketReadPointer = gbPacketReadPointer & 0x1F;
		return c;
	}
	return 0;
}
char* readSerialLine() {
	char c, i;
	i = 0;
	char* line = "";
	while (i < 128) {
		c = readSerialChar();
		if (c == '\r' || c == '\n')
			break;
		else {
			line = line + c;
			i++;
		}
	}
	return line;

}
void printLine(const char* string) {
	printString('\n' + string);
}
void printString(const char* string) {
	int n = 0;
	char c;
	while (string[n]) {
		c = string[n++];
		if (c == '\n') {
			USART_SendData(USART3, (uint8) '\r');
			while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
			USART_SendData(USART3, (uint8) '\n');
			while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
		} else {
			USART_SendData(USART3, (uint8) c);
			while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
		}

	}
}

/**********************************************	 Utility functions	 **************************************************/
void _delay_ms(int time) {
	mDelay((uint32)time);
}
int htoi(char digit) {
	if (digit >= 'A' && digit <= 'F') {
		return digit - 'A' + 10;
	} else if (digit >= 'a' && digit <= 'f') {
		return digit - 'a' + 10;
	} else if (digit >= '0' && digit <= '9') {
		return digit - '0';
	}

	return -1; // Bad input.
}

void printID(uint8 id) {
	char vals[30][2] = {
		"01", "02", "03", "04", "05", "06",
		"07", "08", "09", "10", "11", "12",
		"13", "14", "15", "16", "17", "18",
		"19", "20", "21", "22", "23", "24",
		"25", "26", "27", "28", "29", "30"
	};
		// For some reason PrintString(vals[id]) will print EVERY string in the array until id == (int) i is reached. Weird.
	PrintChar(vals[id][0]);
	PrintChar(vals[id][1]);
}
void end() {
	while (giBusUsing);			// Wait for the bus to flush.
	PrintLine("End of Program.\n");
	setLEDsOff();						// Turn off all LEDs
	/**
	 uint8 time = 0;
	 bool pressed = false;
	 while (1) {
	 if (ReadButton(START)) {
	 pressed = true;
	 // Could use mDelay(time), but this is a more comprehensive method of doing it.
	 while (pressed) {								// Wait until the button isn't pressed, or 1s has passed.
	 if (ReadButton(START))				// If it's still pressed
	 time++;											// Add one (second) to time.
	 else													// Otherwise make sure to reset the flag
	 pressed = false;						// and break the loop.
	 mDelay(1);										// pause for 1ms.
	 }
	 if (time > 750)											// If it was pressed for at least 3/4 of a second.
	 pressed = true;
	 if (pressed) {
	 blinkLED(POWER);
	 pressed = false;
	 }
	 }
	 }
	 **/
	// Just flash the Power LED on and off
	while (1) {
		GPIO_ResetBits(GPIOC, (uint16)0x2000);
		mDelay(750);
		GPIO_SetBits(GPIOC, (uint16)0x2000);
		mDelay(750);
	}
}
/********************************************	 Gizmo setup functions	 ************************************************/
	// 1 May 2013

void gizmo_init() {
	Gizmo.left.arm.shoulder = 4;		// X
	Gizmo.left.arm.elbow = 6;				// X
	Gizmo.left.arm.hand = 8;				// X
	Gizmo.left.leg.hip = 12;				// X
	Gizmo.left.leg.knee = 13;				// X
	Gizmo.left.leg.foot = 14;				// X
	Gizmo.right.arm.shoulder = 3;		// X
	Gizmo.right.arm.elbow = 5;			// X
	Gizmo.right.arm.hand = 7;				// X
	Gizmo.right.leg.hip = 11;				// X
	Gizmo.right.leg.knee = 14;			// X
	Gizmo.right.leg.foot = 16;			// X
}
void setGizmoLeftArm(uint8 shoulder, uint8 elbow, uint8 hand) {
	Gizmo.left.arm.shoulder = shoulder;
	Gizmo.left.arm.elbow = elbow;
	Gizmo.left.arm.hand = hand;
}
void setGizmoLeftLeg(uint8 hip, uint8 knee, uint8 foot) {
	Gizmo.left.leg.hip = hip;
	Gizmo.left.leg.knee = knee;
	Gizmo.left.leg.foot = foot;
}
void setGizmoRightArm(uint8 shoulder, uint8 elbow, uint8 hand) {
	Gizmo.right.arm.shoulder = shoulder;
	Gizmo.right.arm.elbow = elbow;
	Gizmo.right.arm.hand = hand;
}
void setGizmoRightLeg(uint8 hip, uint8 knee, uint8 foot) {
	Gizmo.right.leg.hip = hip;
	Gizmo.right.leg.knee = knee;
	Gizmo.right.leg.foot = foot;
}
/*******************************************	Helpful & neat LED Functions	*******************************************/
	// 1 April 2013
void led_init() {
	setLED(TXD, 0);
	setLED(RXD, 0);
	setLED(AUX, 0);
	setLED(MANAGE, 0);
	setLED(PROGRAM, 0);
	setLED(PLAY, 0);
}
void blinkLEDs() {
	setLEDsOff();	// Set the LEDs off in case any of them happen to be on.
	setLEDsOn();
	mDelay(500);	// Wait 0.5s
	setLEDsOff();
	mDelay(1000);
}
void blinkMultiLEDs(uint8* leds) {
	uint8 size = sizeof(leds) / sizeof(uint8);
	uint8 i = 0;
	for (i = 0; i < size; i++)				// Check each led value in case it's not a valid LED, and turn each one off.
		if (leds[i] > 6) return;
	setLEDsOff();
	for (i = 0; i < size; i++)
		GPIO_ResetBits(EasyLED[leds[i]].port, EasyLED[leds[i]].pin);
	mDelay(150);
	setLEDsOff();
}
void setLED(LED_t led, uint8 state) {
	if (state != 0 || state != 1) return;
		// My normal amount of paranoia for people not sending the correct values.
	switch (state) {
		case 0:	GPIO_SetBits(EasyLED[led].port, EasyLED[led].pin);		break;
		case 1:	GPIO_ResetBits(EasyLED[led].port, EasyLED[led].pin);	break;
	}
}
void setLEDsOff() {
	GPIO_SetBits(EasyLED[0].port, EasyLED[0].pin);
	GPIO_SetBits(EasyLED[1].port, EasyLED[1].pin);
	GPIO_SetBits(EasyLED[2].port, EasyLED[2].pin);
	GPIO_SetBits(EasyLED[3].port, EasyLED[3].pin);
	GPIO_SetBits(EasyLED[4].port, EasyLED[4].pin);
	GPIO_SetBits(EasyLED[5].port, EasyLED[5].pin);
	GPIO_SetBits(EasyLED[6].port, EasyLED[6].pin);
}
void setLEDsOn() {
	GPIO_ResetBits(EasyLED[0].port, EasyLED[0].pin);
	GPIO_ResetBits(EasyLED[1].port, EasyLED[1].pin);
	GPIO_ResetBits(EasyLED[2].port, EasyLED[2].pin);
	GPIO_ResetBits(EasyLED[3].port, EasyLED[3].pin);
	GPIO_ResetBits(EasyLED[4].port, EasyLED[4].pin);
	GPIO_ResetBits(EasyLED[5].port, EasyLED[5].pin);
	GPIO_ResetBits(EasyLED[6].port, EasyLED[6].pin);
}
void setLEDs(bool state) {
	if (state)
		setLEDsOn();
	else
		setLEDsOff();
}
/*******************************************		Dynamixel Servo functions		*******************************************/
	// 7 April 2013
	// Dynmixel config functions
void dxl_init() {
	PrintString("\nInitializing Dynamixels... ");
	USART_Configuration(USART_DXL, 1);
	setLED(TXD, 1);																// Set the TxD LED to On.
	if (dxl_initialize(1))												// Initialize the bus
		PrintString("Initialized.\n");
	else
		PrintString("Unable to Initialize DXLs");
	setLED(TXD, 0);																// Turn off the TxD LED
	mDelay(50);
}
void verbose_ping_all_dxl() {
	uint8 i = 0;
	for (i = 0; i < NUM_AX12_SERVOS; i++) {
		verbose_ping_dxl(i);
		PrintString("\n");
	}
}
void verbose_ping_dxl(uint8 id) {
		// This is a VERY verbose function that was created to detail everything that happens when creating, sending,
		// and receiving packets to Dynamixel servos in order to debug it and/or it's connectivity.
		// Don't use this frequently. It's bulky and not very memory-efficient.
	PrintString("Waiting on bus to clear... ");
	while (giBusUsing);
	PrintString("Bus cleared\nClearing packet... ");
	dxl_clear_statpkt();														// Clear the packet.
	PrintString("Cleared\nSetting new packet ID, Len, & Inst... ");
	dxl_set_txpacket_id(id);												// Set the ID that will receive the packet.
	dxl_set_txpacket_instruction(INST_PING);				// Set the type of instruction that the packet is containing.
	dxl_set_txpacket_length(2);											// Set the length of the packet.
	PrintString("Done\nPinging... ");
	setLED(AUX, 1);
	dxl_txrx_packet();															// Transmit the packet.
	//mDelay(500);																		// Wait for half a second
	setLED(AUX, 0);																	// Now turn off the LED.
	//mDelay(25);
		// If we don't time out do this stuff.
	printStatus();
	PrintString("Clearing packet... ");
	dxl_clear_statpkt();
	PrintString("Cleared.\n\n");

}
void ping_dxl(uint8 id) {
	id++;																												// The DXLs accept 1-n, not 0-n. Add 1 to get a good value
	while (giBusUsing);
	dxl_clear_statpkt();																				// Clear the packet.
	dxl_set_txpacket_id(id);																		// Set the ID that will receive the packet.
	dxl_set_txpacket_instruction(INST_PING);										// Set the type of instruction the packet is containing.
	dxl_set_txpacket_length(2);																	// Set the length of the packet.
	setLED(AUX, 1);
	PrintLine("DXL-");
	printID(id - 1);
	PrintString(" Ping Status: ");
	dxl_txrx_packet();																					// Transmit the packet.
	int time = 1000;																						// Set the timeout value to 2.5s (1s = 1000ms)
	while (time - 1 > 0 && dxl_get_result() != COMM_RXTIMEOUT) {// Loop while the status isn't timeout, or time is up.
		if (dxl_get_result() == COMM_RXSUCCESS ||
				dxl_get_result() == COMM_TXSUCCESS) break;
		mDelay(1);
		time--;
	}
	if (dxl_get_result() == COMM_RXSUCCESS || dxl_get_result() == COMM_TXSUCCESS)
		PrintString("Success.");
	else {
		if (time < 10)
			PrintString("Timeout.");
		else
			printStatus();
	}
	dxl_clear_statpkt();

}
void dxl_check() {
	PrintString("Checking Dynamixels...\n");
	uint8 id = 0;
	int ping;
	for (id = 0; id < NUM_AX12_SERVOS; id++) {			// Go through each servo listed.
		PrintString("DXL ");													// Print out some
		Printu8h(id+1);																// debugging info via
		PrintString(": ");														// the serial link.
		setLED(TXD, 1);
		ping = dxl_ping(id);
			// If the ping times out or generates an error, let us know by printing a string to the Terminal.
		switch (ping) {
			case COMM_RXSUCCESS:	PrintString("PASS\n"); break;
			case COMM_TXSUCCESS:	PrintString("PASS\n"); break;
			case COMM_RXTIMEOUT:	PrintString("FAIL\n"); break;
			case COMM_TXERROR:		PrintString("ERROR\n");break;
			case COMM_RXFAIL:			PrintString("FAIL\n"); break;
			case COMM_TXFAIL:			PrintString("FAIL\n"); break;
			default:							PrintString("FAIL\n"); break;		// Innocent until proven guilty does not apply here.
		}
		setLED(TXD, 0);
	}
	PrintString("Done checking Dynamixels.\n");
}
void printStatus() {
	uint8 stat = dxl_get_result();
	/*uint8 e_bits[7] = {ERRBIT_VOLTAGE,ERRBIT_ANGLE, ERRBIT_OVERHEAT,
	 ERRBIT_RANGE,ERRBIT_CHECKSUM, ERRBIT_OVERLOAD, ERRBIT_INSTRUCTION
	 };*/
	uint8 e_bit = 0;
	if (stat == COMM_TXSUCCESS || stat == COMM_RXSUCCESS) {
		PrintString("Success\n");
		return;
	} else if (stat == COMM_RXTIMEOUT) {
		PrintString("Timeout\n");
		return;
	}
	if (stat & COMM_TXFAIL)
		PrintString("COMM_TXFAIL: Failed transmit instruction packet!");
	else if (stat & COMM_RXFAIL)
		PrintString("COMM_RXFAIL: Failed get status packet from device!");
	else if (stat & COMM_TXERROR)
		PrintString("COMM_TXERROR: Incorrect instruction packet!");
	else if (stat & COMM_BAD_INST)
		PrintString("COMM_BAD_INST: Invalid Instruction byte");
	else if (stat & COMM_BAD_ID)
		PrintString("COMM_BAD_ID: ID's not same for instruction and status packets");
	else if (stat & COMM_RXWAITING)
		PrintString("COMM_RXWAITING: Now receiving status packet!");
	else if (stat & COMM_RXTIMEOUT)
		PrintString("COMM_RXTIMEOUT: There is no status packet!");
	else if (stat & COMM_RXCHECKSUM)
		PrintString("COMM_RXCHECKSUM: Incorrect status packet checksum!");
	else {
		for (e_bit = ERRBIT_VOLTAGE; e_bit < 64; e_bit = e_bit * 2) {
			switch (dxl_get_rxpacket_error(e_bit)) {
				case 1:
					switch (e_bit) {
						case ERRBIT_VOLTAGE:			PrintString("Input voltage error!\n");		break;
						case ERRBIT_ANGLE:				PrintString("Angle limit error!\n");			break;
						case ERRBIT_OVERHEAT:			PrintString("Overheat error!\n");					break;
						case ERRBIT_RANGE:				PrintString("Out of range error!\n");			break;
						case ERRBIT_CHECKSUM:			PrintString("Checksum error!\n");					break;
						case ERRBIT_OVERLOAD:			PrintString("Overload error!\n");					break;
						case ERRBIT_INSTRUCTION:	PrintString("Instruction code error!\n");	break;
						default:									PrintString("Unknown error code! WHAT DID YOU DO!?"); break;
					}
					return;
					break;
				case 0:
					PrintString(".");
					break;
			}
		}
	}
	PrintString("\n");
}

	// Dynamixel debug functions
void printAllServoInfo() {
	uint8 id = 0;
	for (id = 0; id < NUM_AX12_SERVOS; id++) {
		printServoInfo(id);
		mDelay(1);
	}
}
void printServoInfo(uint8 id) {
	dxl_write_byte(id, DXL_LED, 1);
	PrintString("DXL ID:            ");
	pcu_put_byte(dxl_read_byte(id, 0x03));
		// Transmit the model number of the DXL
	PrintString("DXL Model:         ");
	pcu_put_byte(dxl_read_word(id, dxl_makeword(dxl_read_byte(id, 0x00), dxl_read_byte(id, 0x01))));
		// Transmit the firmware of the DXL
	PrintString("DXL Firmware:      ");
	pcu_put_byte(dxl_read_byte(id, 0x02));
		// Transmit the baudrate used by the DXL
	PrintString("DXL Baudrate:      ");
	pcu_put_byte(dxl_read_byte(id, 0x04));
		// Transmit the delay when sending/receiving data to/from the DXL
	PrintString("Delay:             ");
	pcu_put_byte(dxl_read_byte(id, 0x05));
		// Transmit the status level of the DXL
	PrintString("Status Level:      ");
	pcu_put_byte(dxl_read_byte(id, 0x10));
		// Transmit the current position of the DXL
	PrintString("Current Position:  ");
	pcu_put_byte(dxl_read_word(id,dxl_makeword(dxl_read_byte(id, 0x24), dxl_read_byte(id, 0x25))));
}

	// 15 May 2013 Dynamixel R|W functions
uint8 getDXLFirmware(uint8 id) {
	return dxl_read_byte(id, 0x02);
}
uint8 getDXLBaudrate(uint8 id) {
	return dxl_read_byte(id, 0x04);
}
uint8 getDXLStatus(uint8 id) {
	return dxl_read_byte(id, 0x10);
}
uint8 getDXLTorqueState(uint8 id) {
	return dxl_read_byte(id, 0x18);
}
uint8 getDXLCurrentState(uint8 id) {
	uint16 pos[2];
	pos[0] = getDXLCurrentPosition(id);
	mDelay(10);
	pos[1] = getDXLCurrentPosition(id);
	if (pos[0] != pos[1]) {
		return 1;
	} else {
		return 0;
	}
}
uint8 getDXLCurrentVoltage(uint8 id) {
	return dxl_read_byte(id, 0x2A);
}
uint8 getDXLCurrentTemperature(uint8 id) {
	return dxl_read_byte(id, 0x2B);
}
uint16 getDXLModel(uint8 id) {
	return dxl_makeword(dxl_read_byte(id, 0x00), dxl_read_byte(id, 0x01));
}
uint16 getDXLGoalPosition(uint8 id) {
	return dxl_makeword(dxl_read_byte(id, 0x1E), dxl_read_byte(id, 0x1F));
}
uint16 getDXLCurrentPosition(uint8 id) {
	return dxl_makeword(dxl_read_byte(id, 0x24), dxl_read_byte(id, 0x25));
}
uint16 getDXLCurrentSpeed(uint8 id) {
	return dxl_makeword(dxl_read_byte(id, 0x26), dxl_read_byte(id, 0x27));
}
uint16 getDXLCurrentLoad(uint8 id) {
	return dxl_makeword(dxl_read_byte(id, 0x28), dxl_read_byte(id, 0x29));
}

void setDXLParams() {
	int ping, i;
	uint8 PARAM[7] = {
		DXL_ALARM_LED,
		DXL_ALARM_SHUTDOWN,
		DXL_TEMP_LIMIT,
		DXL_VOLT_LOW_LIMIT,
		DXL_CW_COMP_MARGIN,
		DXL_CCW_COMP_MARGIN,
		DXL_TORQ_ENABLE
	};
	uint8 VALUE[7] = {36, 36, 70, 70, 2, 2, 1};
	for (i = 0; i < sizeof(PARAM)/sizeof(uint8); i++) {
		setLED(MANAGE, 1);
		ping = dxl_write_byte(BROADCAST_ID, PARAM[i], VALUE[i]);
		PrintCommStatus(ping);
		setLED(MANAGE, 0);
		mDelay(25);
	}/*
		ping = dxl_write_byte(BROADCAST_ID, DXL_ALARM_LED, 36);
		if(ping != COMM_RXSUCCESS)		PrintCommStatus(ping);	// Set Alarm LED
		if(dxl_write_byte(BROADCAST_ID, DXL_ALARM_SHUTDOWN, 36) != COMM_RXSUCCESS)	PrintErrorCode();	// Prevent overheating
		if(dxl_write_byte(BROADCAST_ID, DXL_TEMP_LIMIT, 70) != COMM_RXSUCCESS)			PrintErrorCode();	// Set Temperature limits
		if(dxl_write_byte(BROADCAST_ID, DXL_VOLT_LOW_LIMIT, 70) != COMM_RXSUCCESS)	PrintErrorCode();	// Set Voltage limits
		if(dxl_write_byte(BROADCAST_ID, DXL_CW_COMP_MARGIN, 2) != COMM_RXSUCCESS)		PrintErrorCode();	// Set a 2-point
		if(dxl_write_byte(BROADCAST_ID, DXL_CCW_COMP_MARGIN, 2) != COMM_RXSUCCESS)	PrintErrorCode();	// compliance margin
		_delay_ms(100);
		if(dxl_write_byte(BROADCAST_ID, DXL_TORQ_ENABLE, 1) != COMM_RXSUCCESS)			PrintErrorCode();	// Enable torque
		*/
}
void setDXLPosition(uint8 id, int pos) {
	if (pos < 0 || pos > 1023) return;
		// Setting the Goal Position changes the DXL's position, but changing the current won't.
	dxl_write_word(id, 0x1E, pos);
		//dxl_write_word(id, 30, pos);
}
void setDXLTorqueState(uint8 id, bool state) {
	if (state)
		dxl_write_byte(id, 0x18, 1);
	else
		dxl_write_byte(id, 0x18, 0);
}
void setDXLLEDState(uint8 id, bool state) {
	if (state) {
		dxl_write_byte(id, 0x19, 1);
	} else {
		dxl_write_byte(id, 0x19, 0);
	}
}

/** Global DXL Functions to manipulate ALL DXLs. **/
void setTorqueOff() {
	uint8 id = 0;
	for (id = 0; id < 18; id++) {
		dxl_write_byte(id, 0x18, Off);
	}
	TorqueState = FALSE;
}
void setTorqueOn() {
	uint8 id = 0;
	for (id = 0; id < 18; id++) {
		dxl_write_byte(id, 0x18, On);
	}
	TorqueState = TRUE;
}
void setTorque(bool state) {
	uint8 id = 0;
	for (id = 0; id < 18; id++) {
		if (state) dxl_write_byte(id, 0x18, On);
		else dxl_write_byte(id, 0x18, Off);
	}
}
void switchTorque() {
	if (TorqueState) {
		dxl_write_byte(254, 24, 0);
	} else {
		dxl_write_byte(254, 24, 1);
	}
	TorqueState = !TorqueState;
}



/********************************************				USART Functions				*********************************************/
void USART_Configuration(uint8 PORT, uint32 baudrate) {
		//	uint8 USART_DXL = 0;
		//	uint8 USART_PCU = 2;
	if (PORT != 0 || PORT != 2) return;
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);
	PORT == 0 ? USART_InitStructure.USART_BaudRate = Baudrate_DXL:Baudrate_PCU;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	PORT == 0?USART_DeInit(USART1) : USART_DeInit(USART3);
	PORT == 0?USART_ITConfig(USART1, USART_IT_RXNE, ENABLE) : USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	PORT == 0?USART_Cmd(USART1, ENABLE) : USART_Cmd(USART3, ENABLE);
	/*
	 if(PORT == USART_DXL) {
	 USART_DeInit(USART1);
	 mDelay(10);
	 USART_Init(USART1, &USART_InitStructure);					// Configure the USART1
	 USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);		// Enable USART1 Receive and Transmit interrupts
	 // USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	 USART_Cmd(USART1, ENABLE);												// Enable the USAT1
	 } else if (PORT == USART_PCU) {
	 USART_DeInit(USART3);
	 mDelay(10);
	 USART_Init(USART3, &USART_InitStructure);					// Configure the USART3
	 USART_Cmd(USART3, ENABLE);												// Enable the USART3
	 }
	 */
}
void DisableUSART1(void) {
	USART_Cmd(USART1, DISABLE);
}



/**********************************************************************************************************************/


	//#define USING_SYSTICK_100US
#define USING_SYSTICK_10US
	//#define USING_SYSTICK_1US

#define DEBUG_PRINT_VOLTAGE



	// CM-530 Helper functions - NaN


	// Mic - input - (read(mic, mic) != SET) => heard
	// Pull-up?
	// Differential opamp?
	// LED - out - reset(led, led) => ON
	// 5V -> LED -> Resistor -> Pin
	// SW  - input - (read(sw, sw) != SET) => pressed
	// Pull-ups
	// 3.3V -> Resistor -> Pin -> GND

vu32 glBuzzerCounter;

void SetLED(LED_t led, uint8 state) {
	if (state)
		GPIO_ResetBits(EasyLED[led].port, EasyLED[led].pin);
	else
		GPIO_SetBits(EasyLED[led].port, EasyLED[led].pin);
}
uint8 ReadButton(Button_t button) {
	if (GPIO_ReadInputDataBit(EasyButton[button].port, EasyButton[button].pin) != SET)
		return 1;
	return 0;
}

	//volatile uint32 glBuzzerCounter;
void start_countdown_buzzer(uint32);
void Buzzed(uint32 mlength, uint32 tone) {
		// Twelve-Tone Equal Temperment (12-TET)
		//   1 octave is a doubling of frequency and equal to 1200 cents
		//   1 octave => 12 equally distributed notes (12 intervals/semitones)
		//     so 100 cents per note
		// Tuned to A 440 (440Hz), so 100 cents per note relative to A_5 (440Hz)
		// n [cents] = 1200 log2(b/a)
		// b = a * 2^(n/1200)
		// tone = 1/(2*1e-6*f) = 1/(2*1e-6*440*2^(cents_relative/1200))
		//   using uDelay(), 50% duty cycle, cents relative to A_5
		//#define FREQTOTONE(f)    (5000000/f)    // (1/(2*1e-6*f))
	start_countdown_buzzer(mlength);
	while (glBuzzerCounter > 0) {
		GPIO_ResetBits(PORT_BUZZER, PIN_BUZZER);
		uDelay(tone);
		GPIO_SetBits(PORT_BUZZER, PIN_BUZZER);
		uDelay(tone);
	}
}
void PlayNote(uint32 mlength, buzzed_note_t note, uint8 octave) {
	Buzzed(mlength, (uint32) (note >> octave));
}
void SetEPort(EPortD_t pin, uint8 state) {
	if (state)
		GPIO_SetBits(EasyEPort[pin].port, EasyEPort[pin].pin);
	else
		GPIO_ResetBits(EasyEPort[pin].port, EasyEPort[pin].pin);
}

#define ANALOG_RIGHT_BIT_SHIFT          0

uint16 ReadAnalog(EPortA_t port) {
	if ((port == EPORT1A) || (port == EPORT4A)) {
			// Select EPORT1A and EPORT4A via multiplexer
		GPIO_ResetBits(PORT_ADC_SELECT0, PIN_ADC_SELECT0);
		GPIO_ResetBits(PORT_ADC_SELECT1, PIN_ADC_SELECT1);
		uDelay(5);
		if (port == EPORT1A) {
				// Start ADC1 Software Conversion
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			uDelay(5);
			return (uint16) (ADC_GetConversionValue(ADC1)) >> ANALOG_RIGHT_BIT_SHIFT;
		} else {
				// Start ADC2 Software Conversion
			ADC_SoftwareStartConvCmd(ADC2, ENABLE);
			uDelay(5);
			return (uint16) (ADC_GetConversionValue(ADC2)) >> ANALOG_RIGHT_BIT_SHIFT;
		}
	} else if ( (port == EPORT2A) || (port == EPORT5A) ) {
			// Select EPORT2A and EPORT5A via multiplexer
		GPIO_SetBits(PORT_ADC_SELECT0, PIN_ADC_SELECT0);
		GPIO_ResetBits(PORT_ADC_SELECT1, PIN_ADC_SELECT1);
		uDelay(5);
		if (port == EPORT2A) {
				// Start ADC1 Software Conversion
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			uDelay(5);
			return (uint16) (ADC_GetConversionValue(ADC1)) >> ANALOG_RIGHT_BIT_SHIFT;
		} else {
				// Start ADC2 Software Conversion
			ADC_SoftwareStartConvCmd(ADC2, ENABLE);
			uDelay(5);
			return (uint16) (ADC_GetConversionValue(ADC2)) >> ANALOG_RIGHT_BIT_SHIFT;
		}
	} else if ( (port == EPORT3A) || (port == EPORT6A) ) {
			// Select EPORT3A and EPORT6A via multiplexer
		GPIO_ResetBits(PORT_ADC_SELECT0, PIN_ADC_SELECT0);
		GPIO_SetBits(PORT_ADC_SELECT1, PIN_ADC_SELECT1);
		uDelay(5);
		if (port == EPORT3A) {
				// Start ADC1 Software Conversion
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			uDelay(5);
			return (uint16) (ADC_GetConversionValue(ADC1)) >> ANALOG_RIGHT_BIT_SHIFT;
		} else {
				// Start ADC2 Software Conversion
			ADC_SoftwareStartConvCmd(ADC2, ENABLE);
			uDelay(5);
			return (uint16) (ADC_GetConversionValue(ADC2)) >> ANALOG_RIGHT_BIT_SHIFT;
		}
	} else if (port == VBUS) {
		uint16 temp;
			// Set ADC1 to read SIG_VDD/VBUS on Channel 13
		ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1 , ADC_SampleTime_239Cycles5);
		uDelay(5);
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		uDelay(5);
		temp = (ADC_GetConversionValue(ADC1)) >> ANALOG_RIGHT_BIT_SHIFT;
			// Set ADC1 to read SIG_ADC0 (ADC1 multiplexer output) on Channel 10
		ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1 , ADC_SampleTime_239Cycles5);
		uDelay(5);
		return temp;
	}
	return 0x8000;
}
uint16 ReadIR(EPortA_t port) {
	uint16 temp;
	SetEPort((port * 2), 1);
	SetEPort((port * 2) + 1, 0);
	uDelay(25);
	temp = ReadAnalog(port);
	SetEPort((port * 2), 0);
	SetEPort((port * 2) + 1, 0);
	return temp;
}
void Battery_Monitor_Alarm(void) {
	uint16 volt = ReadAnalog(VBUS) >> 4;
#ifdef DEBUG_PRINT_VOLTAGE
	PrintString("\nBattery Voltage: ");
	Printu32d(volt);
	PrintString("e-1 [Volts]\n");
#endif
		// ALARM!!!
	if (volt < VBUS_LOW_LIMIT) {
		Buzzed(500, 100);
		Buzzed(500, 5000);
		Buzzed(500, 100);
		Buzzed(500, 5000);
		PrintString("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		PrintString("Battery Voltage Critical");
		PrintString("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	}
	return;
}




	// CM-530 Utility functions
volatile uint32 glDelayCounter;
volatile uint32 glCountdownCounter;
volatile uint32 glDxlTimeoutCounter;
volatile uint32 glPcuTimeoutCounter;
	//volatile uint32 glBatTimeoutCounter;
	//volatile uint32 glBatTimeoutSet;
volatile uint8 gbCounterCount;

void start_timeout_dxl(uint32);
void start_timeout_pcu(uint32);
#ifdef USING_BREAK_TO_BOOTLOADER
void BreakToBootLoader(void);
#endif


void mDelay(uint32 nTime) {
	uDelay(nTime * 1000);
}
void uDelay(uint32 nTime) {
	if (glDelayCounter == 0)
		gbCounterCount++;
		// Due to SysTick() interrupt, default is using 10 [us] intervals
#ifdef USING_SYSTICK_100US
	if (nTime >= 100)
		glDelayCounter = (nTime / 100);
	else
		glDelayCounter = 1;
#elif defined USING_SYSTICK_10US
	if (nTime >= 10)
		glDelayCounter = (nTime / 10);
	else
		glDelayCounter = 1;
#elif defined USING_SYSTICK_1US
	glDelayCounter = (nTime);
#endif
	if (gbCounterCount == 1) {
			// Enable the SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Enable);
	}
	while (glDelayCounter != 0);
}
void StartCountdown(uint32 StartTime) {
	if (glCountdownCounter == 0)
		gbCounterCount++;
		// Want Timer counting in 1 [ms] intervals
#ifdef USING_SYSTICK_100US
	glCountdownCounter = (StartTime * 10);
#elif defined USING_SYSTICK_10US
	glCountdownCounter = (StartTime * 100);
#elif defined USING_SYSTICK_1US
	glCountdownCounter = (StartTime * 1000);
#endif
	if (gbCounterCount == 1) {
			// Enable the SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Enable);
	}
}
void start_countdown_buzzer(uint32 nTime) {
	if (glBuzzerCounter == 0)
		gbCounterCount++;
		// Want Timer counting in 1 [ms] intervals
#ifdef USING_SYSTICK_100US
	glBuzzerCounter = (nTime * 10);
#elif defined USING_SYSTICK_10US
	glBuzzerCounter = (nTime * 100);
#elif defined USING_SYSTICK_1US
	glBuzzerCounter = (nTime * 1000);
#endif
	if (gbCounterCount == 1) {
			// Enable the SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Enable);
	}
}
void start_timeout_dxl(uint32 nTime) {
	if (glDxlTimeoutCounter == 0)
		gbCounterCount++;
		// Due to SysTick() interrupt, default is using 10 [us] intervals
#ifdef USING_SYSTICK_100US
	if (nTime >= 100)
		glDxlTimeoutCounter = (nTime / 100);
	else
		glDxlTimeoutCounter = 1;
#elif defined USING_SYSTICK_10US
	if (nTime >= 10)
		glDxlTimeoutCounter = (nTime / 10);
	else
		glDxlTimeoutCounter = 1;
#elif defined USING_SYSTICK_1US
	glDxlTimeoutCounter = (nTime);
#endif
	if (gbCounterCount == 1) {
			// Enable the SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Enable);
	}
}
void start_timeout_pcu(uint32 nTime) {
	if (glPcuTimeoutCounter == 0)
		gbCounterCount++;
		// Due to SysTick() interrupt, default is using 10 [us] intervals
#ifdef USING_SYSTICK_100US
	if (nTime >= 100)
		glPcuTimeoutCounter = (nTime / 100);
	else
		glPcuTimeoutCounter = 1;
#elif defined USING_SYSTICK_10US
	if (nTime >= 10)
		glPcuTimeoutCounter = (nTime / 10);
	else
		glPcuTimeoutCounter = 1;
#elif defined USING_SYSTICK_1US
	glPcuTimeoutCounter = (nTime);
#endif
	if (gbCounterCount == 1) {
			// Enable the SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Enable);
	}
}

#ifdef USING_BREAK_TO_BOOTLOADER
void BreakToBootLoader(void) {
		//    WWDG_DeInit();
		//    WWDG_SetPrescaler(WWDG_PRESCALER_1);
		//    WWDG_SetWindowValue(0x00);
		//    WWDG_SetCounter(0x40);
}
#endif



	// Dynamixel SDK platform dependent source
#define DXL_BUFFER_LENGTH               256
static volatile uint16 gbDxlWrite = 0, gbDxlRead = 0;
static volatile uint8 gbpDxlBuffer[DXL_BUFFER_LENGTH];


uint8 dxl_hal_open(uint32 baudrate) {
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);									// Initialize the Dynamixel structure.
	USART_InitStructure.USART_BaudRate = baudrate;					// Set various parameters.
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_DeInit(USART1);
	mDelay(10);																							// Wait 0.01s
	USART_Init(USART1, &USART_InitStructure);								// Configure USART1 (dynamixel)
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);					// Enable USART1 (dynamixel) Receive interrupt
	USART_Cmd(USART1, ENABLE);															// Enable USART1 (dynamixel)
	return 1;
}
void dxl_hal_close(void) {
		// Disable USART1 (dynamixel)
	USART_Cmd(USART1, DISABLE);
}
void dxl_hal_clear(void) {
		// Clear communication buffer
	uint16 i;
	for (i = 0; i < DXL_BUFFER_LENGTH; i++)
		gbpDxlBuffer[i] = 0;
	gbDxlRead = 0;
	gbDxlWrite = 0;
}
uint8 dxl_hal_tx(uint8 *pPacket, uint8 numPacket) {
	uint8 i;
	for (i = 0; i < numPacket; i++) {
			// RX Disable
		GPIO_ResetBits(PORT_ENABLE_RXD, PIN_ENABLE_RXD);
			// TX Enable
		GPIO_SetBits(PORT_ENABLE_TXD, PIN_ENABLE_TXD);
		USART_SendData(USART1, pPacket[i]);
		while( USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET );
			// TX Disable
		GPIO_ResetBits(PORT_ENABLE_TXD, PIN_ENABLE_TXD);
			// RX Enable
		GPIO_SetBits(PORT_ENABLE_RXD, PIN_ENABLE_RXD);
	}
	return numPacket;
}
uint8 dxl_hal_rx(uint8 *pPacket, uint8 numPacket) {
	uint8 i;
	for (i = 0; i < numPacket; i++) {
		if (gbDxlRead != gbDxlWrite) {
			pPacket[i] = gbpDxlBuffer[gbDxlRead++];
			if (gbDxlRead > (DXL_BUFFER_LENGTH - 1))
				gbDxlRead = 0;
		} else
			return i;
	}
	return numPacket;
}
void dxl_hal_set_timeout(uint8 NumRcvByte) {
	start_timeout_dxl(NumRcvByte * 30);
}
uint8 dxl_hal_timeout(void) {
	if (glDxlTimeoutCounter == 0)
		return 1;
	else
		return 0;
}
void RxD_DXL_Interrupt(void) {
	uint8 temp;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		temp = USART_ReceiveData(USART1);
	} else
		return;
	if (gbDxlWrite < (DXL_BUFFER_LENGTH - 1)) {
		gbpDxlBuffer[gbDxlWrite++] = temp;
	} else {
		gbpDxlBuffer[gbDxlWrite] = temp;
		gbDxlWrite = 0;
	}
	if (gbDxlRead == gbDxlWrite)
		gbDxlRead++;
	if (gbDxlRead > (DXL_BUFFER_LENGTH - 1))
		gbDxlRead = 0;
}




	// Dynamixel SDK platform independent source


uint8 dxl_initialize(uint8 baudrate) {
	if (dxl_hal_open((2000000 / (baudrate + 1))) == 0)
		return 0;
	gbCommStatus = COMM_RXSUCCESS;
	giBusUsing = 0;
	return 1;
}
void dxl_terminate() {
	dxl_hal_close();
}
void dxl_tx_packet() {
	uint8 i;
	uint8 TxNumByte, RealTxNumByte;
	uint8 checksum = 0;
	if (giBusUsing == 1)
		return;
	giBusUsing = 1;
	gbCommStatus = 0;
	if (gbInstructionPacket[DXL_PKT_LEN] > (DXL_MAXNUM_TXPARAM + 2)) {
		gbCommStatus |= COMM_TXERROR;
		giBusUsing = 0;
		return;
	}
	if (   (gbInstructionPacket[DXL_PKT_INST] != INST_PING)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_READ_DATA)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_WRITE_DATA)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_REG_WRITE)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_ACTION)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_RESET)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_SYNC_WRITE)
			&& (gbInstructionPacket[DXL_PKT_INST] != INST_CAP_REGION) ) {
		gbCommStatus |= COMM_BAD_INST;
		giBusUsing = 0;
		return;
	}
	gbInstructionPacket[0] = 0xFF;
	gbInstructionPacket[1] = 0xFF;
	for (i = 0; i < (gbInstructionPacket[DXL_PKT_LEN] + 1); i++)
		checksum += gbInstructionPacket[i + 2];
	gbInstructionPacket[gbInstructionPacket[DXL_PKT_LEN] + 3] = ~checksum;
	if (gbCommStatus & (COMM_RXFAIL | COMM_RXTIMEOUT | COMM_RXCHECKSUM | COMM_RXLENGTH | COMM_BAD_INST | COMM_BAD_ID)) {
		dxl_hal_clear();
	}
	TxNumByte = gbInstructionPacket[DXL_PKT_LEN] + 4;
	RealTxNumByte = dxl_hal_tx((uint8*)gbInstructionPacket, TxNumByte);
	if (TxNumByte != RealTxNumByte) {
		gbCommStatus |= COMM_TXFAIL;
		giBusUsing = 0;
		return;
	}
	if (gbInstructionPacket[DXL_PKT_INST] == INST_READ_DATA)
		dxl_hal_set_timeout(gbInstructionPacket[DXL_PKT_PARA + 1] + 6);
	else
		dxl_hal_set_timeout(6);
	gbCommStatus = COMM_TXSUCCESS;
}
void dxl_rx_packet() {
	uint8 i, j, nRead;
	uint8 checksum = 0;
	if (giBusUsing == 0) return;
	giBusUsing = 1;
	if (gbInstructionPacket[0x02] == 254) {
		gbCommStatus = COMM_RXSUCCESS;
		giBusUsing = 0;
		return;
	}
	if (gbCommStatus & COMM_TXSUCCESS) {
		gbRxGetLength = 0;
		gbRxPacketLength = 6;
	}
	nRead = dxl_hal_rx((uint8*)&gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength);
	gbRxGetLength += nRead;
	if (gbRxGetLength < gbRxPacketLength) {
		if (dxl_hal_timeout() == 1) {
			if (gbRxGetLength == 0) gbCommStatus = COMM_RXTIMEOUT;
			else gbCommStatus = COMM_RXLENGTH;
			giBusUsing = 0;
			return;
		}
	}
		// Find packet header
	for (i = 0; i < (gbRxGetLength - 1); i++) {
		if ((gbStatusPacket[i] == 0xFF) && (gbStatusPacket[i + 1] == 0xFF)) break;
		else if ((i == gbRxGetLength - 2) && (gbStatusPacket[gbRxGetLength - 1] == 0xFF)) break;
	}
	if (i > 0) {
		for (j = 0; j < (gbRxGetLength - i); j++) gbStatusPacket[j] = gbStatusPacket[j + i];
		gbRxGetLength -= i;
	}
		// Check if received full packet
	if (gbRxGetLength < gbRxPacketLength) {
		gbCommStatus = COMM_RXWAITING;
		return;
	}
		// Check id pairing
	if (gbInstructionPacket[0x02] != gbStatusPacket[0x02]) {
		gbCommStatus = COMM_BAD_ID | COMM_RXFAIL;
		giBusUsing = 0;
		return;
	}
	gbRxPacketLength = gbStatusPacket[0x03] + 4;
	if (gbRxGetLength < gbRxPacketLength) {
		nRead = dxl_hal_rx((uint8*)&gbStatusPacket[gbRxGetLength], gbRxPacketLength - gbRxGetLength);
		gbRxGetLength += nRead;
		if (gbRxGetLength < gbRxPacketLength) {
			gbCommStatus = COMM_RXWAITING;
			return;
		}
	}
		// Check checksum
	for (i = 0; i < (gbStatusPacket[0x03] + 1); i++) checksum += gbStatusPacket[i + 2];
	checksum = ~checksum;
	if (gbStatusPacket[gbStatusPacket[0x03] + 3] != checksum) {
		gbCommStatus = COMM_RXCHECKSUM | COMM_RXFAIL;
		giBusUsing = 0;
		return;
	}
	gbCommStatus = COMM_RXSUCCESS;
	giBusUsing = 0;

}
void dxl_txrx_packet() {
		//int t = 0;
	dxl_tx_packet();
	if (!(gbCommStatus & COMM_TXSUCCESS)) return;
	dxl_clear_statpkt();
	do {
		dxl_rx_packet();
		uDelay(50);		// mDelay(.05);
									//t += 5;
	} while (gbCommStatus & COMM_RXWAITING);// || t <= 4999);
																					//if (t >= 5000) gbCommStatus = COMM_RXTIMEOUT;

}
uint16 dxl_get_result() {
	return gbCommStatus;
}
/**	set_tx_packet functions by Sam Watkins	**/
void dxl_set_txpacket_id(uint8 id) {
	gbInstructionPacket[DXL_PKT_ID] = id;
}
void dxl_set_txpacket_instruction(uint8 instruction) {
	gbInstructionPacket[DXL_PKT_INST] = instruction;
}
void dxl_set_txpacket_parameter(uint8 index, uint8 value ) {
	gbInstructionPacket[DXL_PKT_PARA + index] = value;
}
void dxl_set_txpacket_length(uint8 length) {
	gbInstructionPacket[DXL_PKT_LEN] = length;
}
/********************************************/
uint8 dxl_get_rxpacket_error(uint8 errbit) {
	if ((gbCommStatus & COMM_RXFAIL))
		return 0x80;
	if (gbStatusPacket[DXL_PKT_ERR]&errbit)
		return 1;
	return 0;
}
uint8 dxl_get_rxpacket_length() {
	if ((gbCommStatus & COMM_RXFAIL))
		return 0;
	return gbStatusPacket[DXL_PKT_LEN];
}
uint8 dxl_get_rxpacket_parameter(uint8 index) {
	if ((gbCommStatus & COMM_RXFAIL))
		return 0;
	return gbStatusPacket[DXL_PKT_PARA + index];
}
uint16 dxl_makeword(uint8 lowbyte, uint8 highbyte) {
	uint16 word;
	word = highbyte;
	word = word << 8;
	word = word + lowbyte;
	return word;
}
/**
 *	Edits by Sam Watkins 2013:
 *	All Read/Write functions were edited to set the LED of the DXL servo that is being accessed
 *	to turn on whenever it is being accessed, and to turn it off when the function completes.
 **/
uint8 dxl_get_lowbyte(uint16 word) {
	uint16 temp = (word & 0x00FF);
	return (uint8) temp;
}
uint8 dxl_get_highbyte(uint16 word) {
	uint16 temp = ((word & 0xFF00) >> 8);
	return (uint8) temp;
}
uint16 dxl_ping(uint8 id) {
	while (giBusUsing);
	setDXLLEDState(id, 1);
	gbInstructionPacket[0x02] = id;
	gbInstructionPacket[0x04] = 0x01;
	gbInstructionPacket[0x03] = 2;
	dxl_txrx_packet();
	mDelay(25);															// Wait for .025s to help the system bus flush
	setDXLLEDState(id, 0);
	return dxl_get_result();
}
uint8 dxl_read_byte(uint8 id, uint8 address) {
	setDXLLEDState(id, 1);
	while(giBusUsing);
	gbInstructionPacket[DXL_PKT_ID] = id;
	gbInstructionPacket[DXL_PKT_INST] = INST_READ_DATA;
	gbInstructionPacket[DXL_PKT_PARA] = address;
	gbInstructionPacket[DXL_PKT_PARA + 1] = 1;
	gbInstructionPacket[DXL_PKT_LEN] = 4;
	dxl_txrx_packet();
	setDXLLEDState(id, 0);
	if ((gbCommStatus & COMM_RXFAIL))
		return 0;
	return gbStatusPacket[DXL_PKT_PARA];
}
int dxl_write_byte(uint8 id, uint8 address, uint8 value) {
	setDXLLEDState(id, 1);
	while(giBusUsing);
	gbInstructionPacket[0x02] = id;
	gbInstructionPacket[0x04] = INST_WRITE_DATA;
	gbInstructionPacket[DXL_PKT_PARA] = address;
	gbInstructionPacket[DXL_PKT_PARA + 1] = value;
	gbInstructionPacket[0x03] = 4;
	dxl_txrx_packet();
	setDXLLEDState(id, 0);
	return gbCommStatus;
}
uint16 dxl_read_word(uint8 id, uint8 address) {
	setDXLLEDState(id, 1);
	while (giBusUsing);
	gbInstructionPacket[DXL_PKT_ID] = id;
	gbInstructionPacket[DXL_PKT_INST] = INST_READ_DATA;
	gbInstructionPacket[DXL_PKT_PARA] = address;
	gbInstructionPacket[DXL_PKT_PARA + 1] = 2;
	gbInstructionPacket[DXL_PKT_LEN] = 4;
	dxl_txrx_packet();
	setDXLLEDState(id, 0);
	if ((gbCommStatus & COMM_RXFAIL))
		return 0;
	return dxl_makeword(gbStatusPacket[DXL_PKT_PARA], gbStatusPacket[DXL_PKT_PARA + 1]);
}
void dxl_write_word(uint8 id, uint8 address, uint16 value) {
	setDXLLEDState(id, 1);
#ifdef DEBUGMODE
	if (giBusUsing) PrintString("Attempting to write_word, but bus in use.");
#endif
	while (giBusUsing);
#ifdef DEBUGMODE
	PrintString("Done waiting");
#endif
	gbInstructionPacket[DXL_PKT_ID] = id;
	gbInstructionPacket[DXL_PKT_INST] = INST_WRITE_DATA;
	gbInstructionPacket[DXL_PKT_PARA] = address;
	gbInstructionPacket[DXL_PKT_PARA + 1] = dxl_get_lowbyte(value);
	gbInstructionPacket[DXL_PKT_PARA + 2] = dxl_get_highbyte(value);
	gbInstructionPacket[DXL_PKT_LEN] = 5;
	dxl_txrx_packet();
	setDXLLEDState(id, 0);
}
void dxl_clear_statpkt(void) {
	uint8 i, max = gbStatusPacket[DXL_PKT_LEN];
	if ( (max > 0) && (max < DXL_MAXNUM_RXPARAM) ) {
		for (i = 0; i < (max + 4); i++)
			gbStatusPacket[i] = 0;
	} else {
		for (i = 0; i < 6; i++)
			gbStatusPacket[i] = 0;
	}
}
void dxl_capture(uint8 id) {
	setDXLLEDState(id, 1);
		//    while(giBusUsing);
		//    gbInstructionPacket[DXL_PKT_ID] = id;
		//    gbInstructionPacket[DXL_PKT_INST] = INST_CAP_REGION;
		//    gbInstructionPacket[DXL_PKT_LEN] = 2;
		//    dxl_txrx_packet();
	dxl_write_byte(id, 0, 0);
	setDXLLEDState(id, 0);
}

#ifdef USING_PC_UART


	// Serial/PC_UART platform dependent source
#define PC_UART_BUFFER_LENGTH           128

static volatile uint16 gbPcuWrite, gbPcuRead;
static volatile uint8 gbpPcuBuffer[PC_UART_BUFFER_LENGTH] = {0};
static volatile uint8 ReBootToBootLoader;
uint8 pcu_hal_open(uint32 baudrate);
void pcu_hal_close(void);
void pcu_hal_set_timeout(uint8 NumRcvByte);
uint8 pcu_hal_timeout(void);
uint8 pcu_get_queue(void);
uint8 pcu_peek_queue(void);
	//uint8 pcu_get_qstate(void);
void pcu_clear_queue(void);
void pcu_put_queue(void);
void RxD_PCU_Interrupt(void);
	//static FILE *PC_UART_Device;
uint8 pcu_hal_open(uint32 baudrate) {
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_DeInit(USART3);
	mDelay(10);
		// Configure USART3 (PC_UART)
	USART_Init(USART3, &USART_InitStructure);
		// Enable USART3 (PC_UART) Receive interrupt
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		// Enable USART3 (PC_UART)
	USART_Cmd(USART3, ENABLE);
		//    PC_UART_Device = fdevopen( std_putchar, std_getchar );
	return 1;

}
void pcu_hal_close(void) {
		// Disable USART3 (PC UART)
	USART_Cmd(USART3, DISABLE);
}
void pcu_hal_set_timeout(uint8 NumRcvByte) {
		// 200us; ~180 us to transmit one byte at 57600 bps
	start_timeout_pcu(NumRcvByte * 200);
}
uint8 pcu_hal_timeout(void) {
	if (glPcuTimeoutCounter == 0)
		return 1;
	else
		return 0;
}
void pcu_put_byte(uint8 bTxdData) {
	SetLED(TXD, 1);
	USART_SendData(USART3, bTxdData);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
	SetLED(TXD, 0);
}
uint8 pcu_get_queue(void) {
	if (gbPcuWrite == gbPcuRead)
		return 0xFF;
	uint8 data = gbpPcuBuffer[gbPcuRead++];
	if (gbPcuRead > (PC_UART_BUFFER_LENGTH - 1))
		gbPcuRead = 0;
	return data;
}
uint8 pcu_peek_queue(void) {
	if (gbPcuWrite == gbPcuRead)
		return 0xFF;
	uint8 data = gbpPcuBuffer[gbPcuRead];
	return data;
}
void pcu_put_queue(void) {
	uint8 temp;
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
		temp = USART_ReceiveData(USART3);
		if (temp == '#')
			ReBootToBootLoader++;
		else
			ReBootToBootLoader = 0;
		if (ReBootToBootLoader > 15) {
#ifdef USING_BREAK_TO_BOOTLOADER
				//BreakToBootLoader();
#endif
		}
	} else
		return;
	SetLED(RXD, 1);
	if (gbPcuWrite < (PC_UART_BUFFER_LENGTH - 1)) {
		gbpPcuBuffer[gbPcuWrite++] = temp;
	} else {
		gbpPcuBuffer[gbPcuWrite] = temp;
		gbPcuWrite = 0;
	}
	if (gbPcuRead == gbPcuWrite)
		gbPcuRead++;
	if (gbPcuRead > (PC_UART_BUFFER_LENGTH - 1))
		gbPcuRead = 0;
	SetLED(RXD, 0);
}
void pcu_clear_queue(void) {
	gbPcuWrite = 0;
	gbPcuRead = 0;
}
uint8 pcu_get_qstate(void) {
	if (gbPcuWrite == gbPcuRead) {
		pcu_clear_queue();
		return 0;
	} else if (gbPcuRead < gbPcuWrite)
		return (uint8) (gbPcuWrite - gbPcuRead);
	else
		return (uint8) (PC_UART_BUFFER_LENGTH - (gbPcuRead - gbPcuWrite));
}
void RxD_PCU_Interrupt(void) {
	pcu_put_queue();
}


	// PC UART platform independent source
uint8 pcu_initialize(uint32 baudrate) {
	if (pcu_hal_open(baudrate) == 0)
		return 0;
	return 1;
}
void pcu_terminate(void) {
	pcu_hal_close();
}
int std_putchar(char c) {
	if (c == '\n') {
		pcu_put_byte((uint8) '\r'); //0x0D
		pcu_put_byte((uint8) '\n'); //0x0A
	} else {
		pcu_put_byte((uint8) c);
	}
	return c;
}
int std_puts(const char *str) {
	int n = 0;
	while (str[n])
		std_putchar(str[n++]);
	return n;
}
int std_getchar(void) {
	char c;
	pcu_hal_set_timeout(10);
	while ( (pcu_hal_timeout() == 0) && (pcu_get_qstate() == 0) );
	if (pcu_get_qstate() == 0)
		return 0xFF;
	c = pcu_get_queue();
	if (c == '\r')
		c = '\n';
	return c;
}
char* std_gets(char *str) {
	uint8 c, len = 0;
	while (len < 128) {
		pcu_hal_set_timeout(10);
		while ( (pcu_hal_timeout() == 0) && (pcu_get_qstate() == 0) );
		if (pcu_get_qstate() == 0) {
			if (len == 0) {
				return 0;//NULL;
			} else {
				str[len] = '\0';
				return str;
			}
		}
		c = pcu_get_queue();
		if ( (c == '\n') || (c == '\0') ) {
			if (len == 0) {
				return 0;//NULL;
			} else {
				str[len] = '\0';
				return str;
			}
		} else
			str[len++] = (int8) c;
	}
	return str;
}
void PrintCommStatus(uint16 Status) {
	if (Status & COMM_TXFAIL)
		std_puts("COMM_TXFAIL: Failed transmit instruction packet!\n");
	if (Status & COMM_RXFAIL)
		std_puts("COMM_RXFAIL: Failed get status packet from device!\n");
	if (Status & COMM_TXERROR)
		std_puts("COMM_TXERROR: Incorrect instruction packet!\n");
	if (Status & COMM_BAD_INST)
		std_puts("COMM_BAD_INST: Invalid Instruction byte\n");
	if (Status & COMM_BAD_ID)
		std_puts("COMM_BAD_ID: ID's not same for instruction and status packets\n");
	if (Status & COMM_RXWAITING)
		std_puts("COMM_RXWAITING: Now receiving status packet!\n");
	if (Status & COMM_RXTIMEOUT)
		std_puts("COMM_RXTIMEOUT: There is no status packet!\n");
	if (Status & COMM_RXCHECKSUM)
		std_puts("COMM_RXCHECKSUM: Incorrect status packet checksum!\n");
		//    else
		//        std_puts("\nThis is unknown error code!\n");
}
void PrintErrorCode(void) {
	uint8 bits[7] = {ERRBIT_VOLTAGE,ERRBIT_ANGLE, ERRBIT_OVERHEAT,
		ERRBIT_RANGE,ERRBIT_CHECKSUM, ERRBIT_OVERLOAD, ERRBIT_INSTRUCTION
	};
	uint8 bit = 0;
	for (bit = 0; bit < 7; bit++)
		if (dxl_get_rxpacket_error(bits[bit]) == 1) {
			switch (bit) {
				case ERRBIT_VOLTAGE:			PrintString("Input voltage error!\n");		break;
				case ERRBIT_ANGLE:				PrintString("Angle limit error!\n");			break;
				case ERRBIT_OVERHEAT:			PrintString("Overheat error!\n");					break;
				case ERRBIT_RANGE:				PrintString("Out of range error!\n");			break;
				case ERRBIT_CHECKSUM:			PrintString("Checksum error!\n");					break;
				case ERRBIT_OVERLOAD:			PrintString("Overload error!\n");					break;
				case ERRBIT_INSTRUCTION:	PrintString("Instruction code error!\n");	break;
				default: break;
			}
			return;
		}
	/*if(dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
	 PrintString("\nInput voltage error!\n");
	 else if(dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
	 PrintString("\nAngle limit error!\n");
	 else if(dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
	 PrintString("\nOverheat error!\n");
	 else if(dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
	 PrintString("\nOut of range error!\n");
	 else if(dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
	 PrintString("\nChecksum error!\n");
	 else if(dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
	 PrintString("\nOverload error!\n");
	 else if(dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
	 PrintString("\nInstruction code error!\n");*/
}
int PrintChar(char c) {
	return std_putchar(c);
}
int PrintLine(const char* string) {
	std_puts("\n");
	mDelay(10);
	int ret = std_puts(string);
	mDelay(25);
	return ret;
}
int PrintString(const char* s) {
	int ret = std_puts(s);
	mDelay(25);						// Give it a little time to clean itself up; .025s should do it.
	return ret;
}
int GetChar(void) {
	return std_getchar();
}
char* GetString(char* s) {
	return std_gets(s);
}
void Printu32d(uint32 lNum) {
	uint32 temp, div = 1000000000;
	char out[11];
	uint8 i, j;
	for (i = 0; i < 10; i++) {
		temp = (char) (lNum / div);
		lNum = (lNum % div);
			//        lNum -= (uint32) (temp*div);
			//        out[i] = (char) (temp&0x0000000F)+0x30;
		out[i] = (char) ((temp & 0x0F) + 0x30);
		div /= 10;
	}
	out[i] = '\0';
	for (i = 0; i < 10; i++) {
		if (out[0] == '0') {
			for (j = 0; j < 10; j++) {
				out[j] = out[j + 1];
				if (out[j] == '\0')
					break;
			}
		}
	}
	std_puts(out);
	return;
}
void Prints32d(int32 lNumS) {
	uint32 temp, lNum, div = 1000000000;
	char out[12];
	uint8 i, j;
	if (lNum < 0) {
		out[0] = '-';
		lNum = (uint32) ((~lNumS) + 1);
	} else {
		out[0] = '+';
		lNum = (uint32) (lNumS);
	}
	for (i = 1; i < 11; i++) {
		temp = (lNum / div);
		lNum = (lNum % div);
			//        lNum -= (uint32) (temp*div);
			//        out[i] = (char) (temp&0x0000000F)+0x30;
		out[i] = (char) ((temp & 0x0F) + 0x30);
		div /= 10;
	}
	out[i] = '\0';
	for (i = 0; i < 11; i++) {
		if (out[0] == '0') {
			for (j = 0; j < 11; j++) {
				out[j] = out[j + 1];
				if (out[j] == '\0')
					break;
			}
		}
	}
	std_puts(out);
	return;
}
void Printu16h(uint16 wNum) {
	char out[7];
	out[0] = '0';
	out[1] = 'x';
	out[6] = '\0';
	out[2] = (char) ((wNum >> 12) & 0x0F) + 0x30;
	if (out[2] > '9')
		out[2] += 7;
	out[3] = (char) ((wNum >> 8) & 0x0F) + 0x30;
	if (out[3] > '9')
		out[3] += 7;
	out[4] = (char) ((wNum >> 4) & 0x0F) + 0x30;
	if (out[4] > '9')
		out[4] += 7;
	out[5] = (char) (wNum & 0x0F) + 0x30;
	if (out[5] > '9')
		out[5] += 7;
	std_puts(out);
	return;
}
void Printu8h(uint8 bNum) {
	char out[5];
	out[0] = '0';
	out[1] = 'x';
	out[4] = '\0';
	out[2] = (char) ((bNum >> 4) & 0x0F) + 0x30;
	if (out[2] > '9')
		out[2] += 7;
	out[3] = (char) (bNum & 0x0F) + 0x30;
	if (out[3] > '9')
		out[3] += 7;
	std_puts(out);
	return;
}

#endif





#ifdef USING_ZIGBEE


	// Zigbee SDK platform dependent source
#define ZIGBEE_BUFFER_LENGTH            64
static volatile uint8 gbZigWrite = 0, gbZigRead = 0;
static volatile uint8 gbpZigBuffer[ZIGBEE_BUFFER_LENGTH];
uint8 zgb_hal_open(uint32);
void zgb_hal_close(void);
uint8 zgb_hal_tx(uint8*, uint8);
uint8 zgb_hal_rx(uint8*, uint8);
void RxD_ZIG_Interrupt(void);

uint8 zgb_hal_open(uint32 baudrate) {
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_DeInit(UART5);
	mDelay(10);
		// Configure UART5 (ZigBee)
	USART_Init(UART5, &USART_InitStructure);
		// Enable UART5 (ZigBee) Receive interrupt
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
		// Enable UART5 (ZigBee)
	USART_Cmd(UART5, ENABLE);
		// De-activate Reset mode of ZIG-110 module
	GPIO_SetBits(PORT_ZIGBEE_RESET, PIN_ZIGBEE_RESET);
	return 1;
}
void zgb_hal_close() {
		// Disable UART5 (ZigBee)
	USART_Cmd(UART5, DISABLE);
		// Activate Reset mode of ZIG-110 module
		//    GPIO_SetBits(PORT_ZIGBEE_RESET, PIN_ZIGBEE_RESET);    // original
	GPIO_ResetBits(PORT_ZIGBEE_RESET, PIN_ZIGBEE_RESET);    // correct?
}
uint8 zgb_hal_tx(uint8 *pPacket, uint8 numPacket) {
	uint8 i;
	for (i = 0; i < numPacket; i++) {
		SetLED(TXD, 1);
		USART_SendData(UART5, pPacket[i]);
		while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET);
		SetLED(TXD, 0);
	}
	return numPacket;
}
uint8 zgb_hal_rx(uint8 *pPacket, uint8 numPacket) {
	uint8 i;
	for (i = 0; i < numPacket; i++) {
		if (gbZigRead != gbZigWrite) {
			pPacket[i] = gbpZigBuffer[gbZigRead++];
			if (gbZigRead > (ZIGBEE_BUFFER_LENGTH - 1))
				gbZigRead = 0;
		} else
			return i;
	}
	return numPacket;
}
void RxD_ZIG_Interrupt() {
	uint8 temp;
	if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) {
		temp = USART_ReceiveData(UART5);
		if (temp == '#')
			ReBootToBootLoader++;
		else
			ReBootToBootLoader = 0;
		if (ReBootToBootLoader > 15) {
#ifdef USING_BREAK_TO_BOOTLOADER
			BreakToBootLoader();
#endif
		}
	} else
		return;
	SetLED(RXD, 1);
	if (gbZigWrite < (ZIGBEE_BUFFER_LENGTH - 1)) {
		gbpZigBuffer[gbZigWrite++] = temp;
	} else {
		gbpZigBuffer[gbZigWrite] = temp;
		gbZigWrite = 0;
	}
	if (gbZigRead == gbZigWrite)
		gbZigRead++;
	if (gbZigRead > (ZIGBEE_BUFFER_LENGTH - 1))
		gbZigRead = 0;
	SetLED(RXD, 0);
}
	// Zigbee SDK platform independent source
#define PACKET_LENGTH                   6
static uint8 gbRcvPacket[PACKET_LENGTH];
static uint8 gbRcvPacketNum;
static uint16 gwRcvData;
static volatile uint8 gbRcvFlag;
uint8 zgb_initialize(uint32 baudrate) {
	if (zgb_hal_open(baudrate) == 0)
		return 0;
	gbRcvFlag = 0;
	gwRcvData = 0;
	gbRcvPacketNum = 0;
	return 1;
}
void zgb_terminate() {
	zgb_hal_close();
}
uint8 zgb_tx_data(uint16 word) {
	uint8 SndPacket[6];
	uint8 lowbyte = (uint8) (word & 0xFF);
	uint8 highbyte = (uint8) ((word >> 8) & 0xFF);
	SndPacket[0] = 0xFF;
	SndPacket[1] = 0x55;
	SndPacket[2] = lowbyte;
	SndPacket[3] = ~lowbyte;
	SndPacket[4] = highbyte;
	SndPacket[5] = ~highbyte;
	if (zgb_hal_tx(SndPacket, 6) != 6)
		return 0;
	return 1;
}
uint8 zgb_rx_check() {
	uint8 RcvNum;
	uint8 checksum;
	uint8 i, j;
	if (gbRcvFlag == 1)
		return 1;
		// Fill packet buffer
	if (gbRcvPacketNum < 6) {
		RcvNum = zgb_hal_rx((uint8*)&gbRcvPacket[gbRcvPacketNum], (6 - gbRcvPacketNum));
		if (RcvNum != -1)
			gbRcvPacketNum += RcvNum;
	}
		// Find header
	if (gbRcvPacketNum >= 2) {
		for (i = 0; i < gbRcvPacketNum; i++) {
			if (gbRcvPacket[i] == 0xFF) {
				if (i <= (gbRcvPacketNum - 2)) {
					if (gbRcvPacket[i + 1] == 0x55)
						break;
				}
			}
		}
		if (i > 0) {
			if (i == gbRcvPacketNum) {
					// Cannot find header
				if (gbRcvPacket[i - 1] == 0xFF)
					i--;
			}
				// Remove data before header
			for (j = i; j < gbRcvPacketNum; j++) {
				gbRcvPacket[j - i] = gbRcvPacket[j];
			}
			gbRcvPacketNum -= i;
		}
	}
		// Verify packet
	if (gbRcvPacketNum == 6) {
		if ( (gbRcvPacket[0] == 0xFF) && (gbRcvPacket[1] == 0x55) ) {
			checksum = ~gbRcvPacket[3];
			if (gbRcvPacket[2] == checksum) {
				checksum = ~gbRcvPacket[5];
				if (gbRcvPacket[4] == checksum) {
					gwRcvData = (uint16) ((gbRcvPacket[4] << 8) & 0xFF00);
					gwRcvData += gbRcvPacket[2];
					gbRcvFlag = 1;
				}
			}
		}
		gbRcvPacket[0] = 0x00;
		gbRcvPacketNum = 0;
	}
	return gbRcvFlag;
}
uint16 zgb_rx_data() {
	gbRcvFlag = 0;
	return gwRcvData;
}
#endif


	// CM-530 Configuration functions
	// Private functions - Anything outside the API file should and will NOT have access.
void ISR_Delay_Base(void);
void SysTick_Configuration(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void ADC_Configuration(void);

void SysInit(void) {
		//WWDG_ClearFlag();					// Clear the WatchDog Early Wakeup interrupt flag
		// System Init MUST go in this order: RCC, NVIC, GPIO, SysTick, ADC, then USART
	ReBootToBootLoader = 0;
	RCC_Configuration();					// System Clocks Configuration
	NVIC_Configuration();					// NVIC Configuration
	GPIO_Configuration();					// GP I/O port Configuration
	SysTick_Configuration();			// System clock count Configuration
	ADC_Configuration();					// Analog to Digital Converter Configuration
	uint16 error = 0, tog = 0;
	setLED(MANAGE, 1);
		//SetLED(MANAGE, 1);
	USART_Configuration(USART_DXL, 1);
	USART_Configuration(USART_PCU, 57600);
#ifdef USING_PC_UART
	mDelay(100); if (!pcu_initialize(Baudrate_PCU)) error|=(1<<0);
#endif
#ifdef USING_ZIGBEE
	mDelay(100); if (!zgb_initialize(Baudrate_ZIG)) error|=(1<<1);
#endif
#ifdef USING_DYNAMIXEL
	mDelay(100); if (!dxl_initialize(Baudrate_DXL)) error|=(1<<2);
#endif

	SetLED(PLAY, (error&(1<<0)));
	SetLED(PROGRAM, (error&(1<<1)));
	SetLED(MANAGE, (error&(1<<2)));

	while (error) {
		SetLED(POWER, tog);
		tog ^= 1;
		mDelay(500);
	}
	setLED(POWER, 1);
	mDelay(100);
	setLED(POWER, 0);
	PrintString("\nInitialized.\n");
}
void ISR_Delay_Base() {
		// User accessible delay counter
	if (glDelayCounter > 1)
		glDelayCounter--;
	else if (glDelayCounter > 0) {
		glDelayCounter--;
		gbCounterCount--;
	}
		// User accessible timeout/countdown counter
	if (glCountdownCounter > 1) {
		glCountdownCounter--;
#ifdef USING_SYSTICK_100US
		if ( (glCountdownCounter & 0x00000200) )
#elif defined USING_SYSTICK_10US
			if ( (glCountdownCounter & 0x00001000) )
#elif defined USING_SYSTICK_1US
				if ( (glCountdownCounter & 0x00010000) )
#endif
					SetLED(AUX, 1);
				else
					SetLED(AUX, 0);
	} else if (glCountdownCounter > 0) {
		SetLED(AUX, 0);
		glCountdownCounter--;
		gbCounterCount--;
	}
		// Buzzer countdown counter
	if (glBuzzerCounter > 1)
		glBuzzerCounter--;
	else if (glBuzzerCounter > 0) {
		glBuzzerCounter--;
		gbCounterCount--;
	}
		// Dynamixel timeout counter
	if (glDxlTimeoutCounter > 1)
		glDxlTimeoutCounter--;
	else if (glDxlTimeoutCounter > 0) {
		glDxlTimeoutCounter--;
		gbCounterCount--;
	}
		// PC UART timeout counter
	if (glPcuTimeoutCounter > 1)
		glPcuTimeoutCounter--;
	else if (glPcuTimeoutCounter > 0) {
		glPcuTimeoutCounter--;
		gbCounterCount--;
	}
		// Battery Monitor timeout counter
		//    if (glBatTimeoutCounter>1)
		//        glBatTimeoutCounter--;
		//    else
		//    {
		//        Battery_Monitor_Alarm();
		//        if (glBatTimeoutSet>100000)
		//            glBatTimeoutCounter = glBatTimeoutSet;
		//        else
		//            glBatTimeoutCounter = 100000;
		//    }
		// If no active counters, disable interrupt
	if (gbCounterCount == 0) {
			// Disable SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Disable);
			// Clear SysTick Counter
		SysTick_CounterCmd(SysTick_Counter_Clear);
	}
}
void SysTick_Configuration() {
#ifdef USING_SYSTICK_100US
		// SysTick end of count event each 100us with input clock equal to 9MHz (HCLK/8, default)
	SysTick_SetReload(900);
#elif defined USING_SYSTICK_10US
		// SysTick end of count event each 10us with input clock equal to 9MHz (HCLK/8, default)
	SysTick_SetReload(90);
#elif defined USING_SYSTICK_1US
		// SysTick end of count event each 1us with input clock equal to 9MHz (HCLK/8, default)
	SysTick_SetReload(9);
#endif
		// Enable SysTick interrupt
	SysTick_ITConfig(ENABLE);
		// Reset Active Counter count
	gbCounterCount = 0;
}
void RCC_Configuration() {
	ErrorStatus HSEStartUpStatus;
		// RCC system reset(for debug purpose)
	RCC_DeInit();
		// Enable HSE
	RCC_HSEConfig(RCC_HSE_ON);
		// Wait till HSE is ready
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS) {
			// Enable Prefetch Buffer
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
			// Flash 2 wait state
		FLASH_SetLatency(FLASH_Latency_2);
			// HCLK = SYSCLK
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
			// PCLK2 = HCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);
			// PCLK1 = HCLK/2
		RCC_PCLK1Config(RCC_HCLK_Div2);
			// PLLCLK = 8MHz * 9 = 72 MHz
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
			// Enable PLL
		RCC_PLLCmd(ENABLE);
			// Wait till PLL is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
		}
			// Select PLL as system clock source
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
			// Wait till PLL is used as system clock source
		while (RCC_GetSYSCLKSource() != 0x08) {
		}
	}
		// Enable peripheral clocks
		// Enable GPIOB and GPIOC clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
		// Enable USART1 Clock (Dynamixel)
#ifdef USING_DYNAMIXEL
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif
#ifdef USING_PC_UART
		// Enable USART3 Clock (PC_UART)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif
#ifdef USING_ZIGBEE
		// Enable UART5 Clock (Zigbee)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
#endif
	PWR_BackupAccessCmd(ENABLE);
}
void NVIC_Configuration() {
	NVIC_InitTypeDef NVIC_InitStructure;
#ifdef  VECT_TAB_RAM
		// Set the Vector Table base location at 0x20000000
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  // VECT_TAB_FLASH
			 // Set the Vector Table base location at 0x08003000
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x3000);
#endif
		// Configure the NVIC Preemption Priority Bits
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		// Enable the USART1 Interrupt (Dynamixel)
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		// Enable the USART3 Interrupt (Serial/PC_UART)
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		// Enable the UART5 Interrupt (Zigbee)
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void GPIO_Configuration() {
		//if (sizeof(port) != sizeof(char[1])) return 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
		// PORTA CONFIG
	GPIO_InitStructure.GPIO_Pin = PIN_SIG_MOT1P | PIN_SIG_MOT1M | PIN_SIG_MOT2P | PIN_SIG_MOT2M  | PIN_SIG_MOT5P | PIN_SIG_MOT5M;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_BUZZER | PIN_ZIGBEE_RESET;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_ADC1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_SW_RIGHT | PIN_SW_LEFT;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// PORTB CONFIG
	GPIO_InitStructure.GPIO_Pin = PIN_LED_AUX | PIN_LED_MANAGE | PIN_LED_PROGRAM | PIN_LED_PLAY;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_ENABLE_TXD | PIN_ENABLE_RXD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_DXL_RXD | PIN_PC_RXD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_DXL_TXD | PIN_PC_TXD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_SW_START;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
		// PORTC CONFIG
	GPIO_InitStructure.GPIO_Pin = PIN_SIG_MOT3P | PIN_SIG_MOT3M | PIN_SIG_MOT4P | PIN_SIG_MOT4M;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_ADC_SELECT0 | PIN_ADC_SELECT1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_LED_POWER | PIN_LED_TXD | PIN_LED_RXD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_ZIGBEE_TXD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_ADC0 | PIN_VDD_VOLT;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_MIC;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIN_SW_UP | PIN_SW_DOWN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
		// PORTD CONFIG
	GPIO_InitStructure.GPIO_Pin = PIN_ZIGBEE_RXD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinRemapConfig( GPIO_Remap_USART1, ENABLE);
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_Disable, ENABLE);
}
void ADC_Configuration() {
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
		// ADC1 configuration
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);
		// ADC1 regular channels configuration
		// Set ADC1 to read SIG_ADC0 (ADC1 multiplexer output) on Channel 10
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1 , ADC_SampleTime_239Cycles5);
		// Set ADC1 to read VBUS on Channel 13
		//    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1 , ADC_SampleTime_239Cycles5);    // SIG_VDD/VBUS
		//ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
		// ADC2 regular channels configuration
		// Set ADC2 to read SIG_ADC1 (ADC2 multiplexer output) on Channel 5
	ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 1, ADC_SampleTime_239Cycles5);    // SIG_ADC1
																																									//ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);
																																									// Enable ADC1 DMA
																																									//ADC_DMACmd(ADC1, ENABLE);
																																									// Enable ADC1,2
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);
		// Enable ADC1,2 reset calibration register
		// Check the end of ADC1,2 reset calibration register
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));
		// Start ADC1,2 calibration
		// Check the end of ADC1,2 calibration
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));
		// Start ADC2 Software Conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}