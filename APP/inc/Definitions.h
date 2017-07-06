//
//  Definitions.h
//  CM530 P04
//
//  Created by Sam Watkins on 5/10/13.
//  Copyright (c) 2013 Sam Watkins. All rights reserved.
//

#ifndef CM530_P04_Definitions_h
#define CM530_P04_Definitions_h

typedef unsigned char		uint8;
typedef unsigned short	uint16;
typedef unsigned long		uint32;

typedef signed char			int8;
typedef signed short		int16;
typedef signed long			int32;

// Instruction Packet (Command Packet)
// 0xFF -> 0xFF -> ID -> LENGTH -> INSTRUCTION -> PARAM_1 -> PARAM_2 -> PARAM_N -> CHECKSUM
// LENGTH = ( 1 {ID Byte} + 1 {INSTRUCTION Byte} + N {Number of PARAMETER Bytes} )
// CHECKSUM = ( ~(ID + LENGTH + INSTRUCTION + PARAM_1 +...+ PARAM_N) ) & (0x00FF)

// Status Packet (Return Packet)
// 0xFF -> 0xFF -> ID -> LENGTH -> ERROR -> PARAM_1 -> PARAM_2 -> PARAM_N -> CHECKSUM
// LENGTH = ( 1 {ID Byte} + 1 {ERROR Byte} + N {Number of PARAMETER Bytes} )
// CHECKSUM = ( ~(ID + LENGTH + ERROR + PARAM_1 +...+ PARAM_N) ) & (0x00FF)



	// Dynamixel Packet byte array offsets
enum {
	DXL_PKT_ID		= 0x02,		// Offset to ID byte of packet
	DXL_PKT_LEN		= 0x03,		// Offset to Length byte of packet
	DXL_PKT_INST	= 0x04,		// Offset to Instruction byte of command packet
	DXL_PKT_ERR		= 0x04,		// Offset to Error byte of return packet
	DXL_PKT_PARA	= 0x05		// Offset to first Parameter byte of packet
};
	// Instruction set PARAMETERS // TASK
enum {
	INST_PING							= 0x01,	// Get status packet.
	INST_READ_DATA				= 0x02,	// Read value from device Control Table.
	INST_WRITE_DATA				= 0x03,	// Write value into device Control Table .
	INST_REG_WRITE				= 0x04,	// Write value in standby mode (wait for REG_WRITE).
	INST_ACTION						= 0x05,	// Execute REG_WRITE.
	INST_RESET						= 0x06,	// Set all Control Table values to default (?)
	INST_DIGITAL_RESET		= 0x07,	// ??? From HaViMo2 - Embedded-C example
	INST_SYSTEM_READ			= 0x0C,	// ??? From HaViMo2 - Embedded-C example
	INST_SYSTEM_WRITE			= 0x0D,	// ??? From HaViMo2 - Embedded C Example
	INST_CAP_REGION				= 0x0E,	// Initiates new image capture and processing, non-responsive during processing
	INST_RAW_SAMPLE				= 0x0F,	// Sample the Raw Image, response doesn't follow standard RoboPlus packet.
	INST_LUT_MANAGE				= 0x10,	// Enter LUT Manage Mode
	INST_RD_FILTHR				= 0x11,	// Read noise threshold value
	INST_WR_FILTHR				= 0x12,	// Write noise threshold value
	INST_RD_REGTHR				= 0x13,	// Read region threshold value (see datasheet for more detail)
	INST_WR_REGTHR				= 0x14,	// Write region threshold value
  INST_CAP_GRID					= 0x15,	// Invokes gridding algorithm
	INST_READ_GRID				= 0x16,	// Retrieves results of gridded image
	INST_SAMPLE_FAST			= 0x17,	// Download an entire image from the module at 1Mbps
  INST_SYNC_WRITE				= 0x83,	// Write value to Control Table of multiple dynamixels
	INST_SYNC_REG_WRITE		= 0x84,	// Write value to Control Table of multiple dynamixels
	INST_BULK_READ				= 0x92,	// Added to CM-730 in version v1.2.0 of the Darwin Framework
	INST_SYNC_READ			  = 0xD0	// Issues INST_READ_DATA cmd for each servo in ID, location, & length of all reads same.
};
	// Error bits of Status Packet's ERROR byte
enum {
	ERRBIT_INSTRUCTION		= 0x40,		// Set to 1 if undefined instruction or register is without REG_WRITE
	ERRBIT_OVERLOAD				= 0x20,		// Set to 1 if specified max torque cannot handle load
	ERRBIT_CHECKSUM				= 0x10,		// Set to 1 if checksum of packet is incorrect
	ERRBIT_RANGE					= 0x08,		// Set to 1 if the instruction sent is out of range
	ERRBIT_OVERHEAT				= 0x04,		// Set to 1 if the module temp is above operating temp defined in control table
	ERRBIT_ANGLE					= 0x02,		// Set to 1 if the Goal position is set out of range
	ERRBIT_VOLTAGE				= 0x01		// Set to 1 if the input voltage is out of range
};
	// Expanded Dynamixel SDK TX/RX Error code bits
enum {
	COMM_TXSUCCESS		= 0x0001,
	COMM_RXSUCCESS		= 0x0002,
	COMM_TXFAIL				= 0x0004,
	COMM_RXFAIL				= 0x0008,
	COMM_TXERROR			= 0x0010,
	COMM_BAD_INST			= 0x0020,
	COMM_BAD_ID				= 0x0040,
	COMM_NULL_DATA		= 0x0080,
	COMM_NULL_SIZE		= 0x0100,
	COMM_RXWAITING		= 0x0200,
	COMM_RXTIMEOUT		= 0x0400,
	COMM_RXLENGTH			= 0x0800,
	COMM_RXCHECKSUM		= 0x1000
};
	// A copy of the COMM Error code bits.
enum {
	DXL_TXSUCCESS		= 0x0001,
	DXL_RXSUCCESS		= 0x0002,
	DXL_TXFAIL			= 0x0004,
	DXL_RXFAIL			= 0x0008,
	DXL_TXERROR			= 0x0010,
	DXL_BAD_INST		= 0x0020,
	DXL_BAD_ID			= 0x0040,
	DXL_NULL_DATA		= 0x0080,
	DXL_NULL_SIZE		= 0x0100,
	DXL_RXWAITING		= 0x0200,
	DXL_RXTIMEOUT		= 0x0400,
	DXL_RXLENGTH		= 0x0800,
	DXL_RXCHECKSUM	= 0x1000
};

	// Default model numbers
#define MODEL_AX12                      12
#define MODEL_AX12W                     300
#define MODEL_AX18                      18
#define MODEL_AXS1                      13
#define MODEL_AXS20                     212
#define MODEL_JHFPS                     17163	// 0x6711
#define MODEL_HaViMo2                   0
#define MODEL_MX28                      29
#define MODEL_IRSA                      330		// 0x014A


	// Universal address table		-- SWW '13: Kept for legacy support
#define P_MODEL_NUMBER_L                0
#define P_MODEL_NUMBER_H                1
#define P_FIRMWARE_VERSION              2
#define P_ID                            3
#define P_BAUD_RATE                     4
#define P_RETURN_DELAY_TIME             5
#define P_STATUS_RETURN_LEVEL           16
	//#define P_LED                           25  // AX-12+, AX-18F, and AX-S20
	//#define P_REGISTERED_INSTRUCTION        44


	// Robotis' IR Sensor Array
enum {
	IRSA_MODEL_NUMBER_L					= 0,
	IRSA_MODEL_NUMBER_H					= 1,
	IRSA_FIRMWARE_VERSION				= 2,
	IRSA_ID											= 3,
	IRSA_BAUD_RATE							= 4,
	IRSA_RETURN_DELAY_TIME			= 5,
	IRSA_ETH_L_1								= 6,
	IRSA_ETH_H_1								= 7,
	IRSA_ETH_L_2								= 8,
	IRSA_ETH_H_2								= 9,
	IRSA_ETH_L_3								= 10,
	IRSA_ETH_H_3								= 11,
	IRSA_ETH_L_4								= 12,
	IRSA_ETH_H_4								= 13,
	IRSA_ETH_L_5								= 14,
	IRSA_ETH_H_5								= 15,
	IRSA_STATUS_RETURN_LEVEL		= 16,
	IRSA_ETH_L_6								= 17,
	IRSA_ETH_H_6								= 18,
	IRSA_ETH_L_7								= 20,
	IRSA_ETH_H_7								= 21,
	IRSA_FD_L_1									= 24,
	IRSA_FD_H_1									= 25,
	IRSA_FD_L_2									= 26,
	IRSA_FD_H_2									= 27,
	IRSA_FD_L_3									= 28,
	IRSA_FD_H_3									= 29,
	IRSA_FD_L_4									= 30,
	IRSA_FD_H_4									= 31,
	IRSA_FD_L_5									= 32,
	IRSA_FD_H_5									= 33,
	IRSA_FD_L_6									= 34,
	IRSA_FD_H_6									= 35,
	IRSA_FD_L_7									= 36,
	IRSA_FD_H_7									= 37,
	IRSA_BUZZ_NOTE							= 40,
	IRSA_BUZZ_TIME							= 41,
	IRSA_AUTO_TH								= 42,
	IRSA_OBS_DET								= 43,
	IRSA_REGISTERED_INSTRUCTION	= 44,
	IRSA_LOCK										= 47,
	IRSA_TH_L_1									= 48,
	IRSA_TH_H_1									= 49,
	IRSA_TH_L_2									= 50,
	IRSA_TH_H_2									= 51,
	IRSA_TH_L_3									= 52,
	IRSA_TH_H_3									= 53,
	IRSA_TH_L_4									= 54,
	IRSA_TH_H_4									= 55,
	IRSA_TH_L_5									= 56,
	IRSA_TH_H_5									= 57,
	IRSA_TH_L_6									= 58,
	IRSA_TH_H_6									= 59,
	IRSA_TH_L_7									= 60,
	IRSA_TH_H_7									= 61
};
	// John Hyland's FSR FOOT PRESSURE SENSOR BOARD
enum {
	JHFPS_MODEL_NUMBER_L              = 0,
	JHFPS_MODEL_NUMBER_H              = 1,
	JHFPS_FIRMWARE_VERSION		= 2,
	JHFPS_ID									= 3,
	JHFPS_BAUD_RATE						= 4,
	JHFPS_RETURN_DELAY_TIME		= 5,
	JHFPS_STATUS_RETURN_LEVEL	= 16,
	JHFPS_LED									= 25,
	JHFPS_FRONT_INSIDE_L			= 26,
	JHFPS_FRONT_INSIDE_H			= 27,
	JHFPS_FRONT_OUTSIDE_L			= 28,
	JHFPS_FRONT_OUTSIDE_H			= 29,
	JHFPS_REAR_INSIDE_L				= 30,
	JHFPS_REAR_INSIDE_H				= 31,
	JHFPS_REAR_OUTSIDE_L			= 32,
	JHFPS_REAR_OUTSIDE_H			= 33
};
	// AX-12+/18F address table
enum {
	AXM_MODEL_NUMBER_L                = 0,
	AXM_MODEL_NUMBER_H                = 1,
	AXM_FIRMWARE_VERSION              = 2,
	AXM_ID                            = 3,
	AXM_BAUD_RATE                     = 4,
	AXM_RETURN_DELAY_TIME             = 5,
	AXM_CW_ANGLE_LIMIT_L              = 6,
	AXM_CW_ANGLE_LIMIT_H              = 7,
	AXM_CCW_ANGLE_LIMIT_L             = 8,
	AXM_CCW_ANGLE_LIMIT_H             = 9,
	AXM_SYSTEM_DATA2                  = 10,
	AXM_HIGHEST_LIMIT_TEMPERATURE     = 11,
	AXM_LOWEST_LIMIT_VOLTAGE          = 12,
	AXM_HIGHEST_LIMIT_VOLTAGE         = 13,
	AXM_MAX_TORQUE_L                  = 14,
	AXM_MAX_TORQUE_H                  = 15,
	AXM_STATUS_RETURN_LEVEL           = 16,
	AXM_ALARM_LED                     = 17,
	AXM_ALARM_SHUTDOWN                = 18,
	AXM_OPERATING_MODE                = 19,
	AXM_DOWN_CALIBRATION_L            = 20,
	AXM_DOWN_CALIBRATION_H            = 21,
	AXM_UP_CALIBRATION_L              = 22,
	AXM_UP_CALIBRATION_H              = 23,
	AXM_TORQUE_ENABLE                 = 24,
	AXM_LED                           = 25,
	AXM_CW_COMPLIANCE_MARGIN          = 26,
	AXM_CCW_COMPLIANCE_MARGIN         = 27,
	AXM_CW_COMPLIANCE_SLOPE           = 28,
	AXM_CCW_COMPLIANCE_SLOPE          = 29,
	AXM_GOAL_POSITION_L               = 30,
	AXM_GOAL_POSITION_H               = 31,
	AXM_MOVING_SPEED_L                = 32,
	AXM_MOVING_SPEED_H                = 33,
	AXM_TORQUE_LIMIT_L                = 34,
	AXM_TORQUE_LIMIT_H                = 35,
	AXM_PRESENT_POSITION_L            = 36,
	AXM_PRESENT_POSITION_H            = 37,
	AXM_PRESENT_SPEED_L               = 38,
	AXM_PRESENT_SPEED_H               = 39,
	AXM_PRESENT_LOAD_L                = 40,
	AXM_PRESENT_LOAD_H                = 41,
	AXM_PRESENT_VOLTAGE               = 42,
	AXM_PRESENT_TEMPERATURE           = 43,
	AXM_REGISTERED_INSTRUCTION        = 44,
	AXM_PAUSE_TIME                    = 45,
	AXM_MOVING                        = 46,
	AXM_LOCK                          = 47,
	AXM_PUNCH_L                       = 48,
	AXM_PUNCH_H                       = 49
};

typedef enum Button_e {
    UP                                = 0,
    DOWN                              = 1,
    LEFT                              = 2,
    RIGHT                             = 3,
    START                             = 4,
    MIC                               = 5
} Button_t;
typedef enum LED_e {
    POWER                             = 0,
    MANAGE                            = 1,
    PROGRAM                           = 2,
    PLAY                              = 3,
    TXD                               = 4,
    RXD                               = 5,
    AUX                               = 6
} LED_t;
typedef enum Motor_e {
    MOTOR1P                           = 0,
    MOTOR1M                           = 1,
    MOTOR2P                           = 2,
    MOTOR2M                           = 3,
    MOTOR3P                           = 4,
    MOTOR3M                           = 5,
    MOTOR4P                           = 6,
    MOTOR4M                           = 7,
    MOTOR5P                           = 8,
    MOTOR5M                           = 9,
    MOTOR6P                           = 10,
    MOTOR6M                           = 11
} Motor_t;
typedef enum EPortD_e {
    EPORT11                           = 0,
    EPORT15                           = 1,
    EPORT21                           = 2,
    EPORT25                           = 3,
    EPORT31                           = 4,
    EPORT35                           = 5,
    EPORT41                           = 6,
    EPORT45                           = 7,
    EPORT51                           = 8,
    EPORT55                           = 9,
    EPORT61                           = 10,
    EPORT65                           = 11
} EPortD_t;
typedef enum EPortA_e {
    EPORT1A                           = 0,
    EPORT2A                           = 1,
    EPORT3A                           = 2,
    EPORT4A                           = 3,
    EPORT5A                           = 4,
    EPORT6A                           = 5,
    VBUS                              = 6
} EPortA_t;
typedef enum buzzed_note_e {
    // Twelve-Tone Equal Temperment (12-TET)
    //   1 octave is a doubling of frequency and equal to 1200 cents
    //   1 octave => 12 equally distributed notes (12 intervals/semitones)
    //     so 100 cents per note
    // Tuned to A 440 (440Hz), so 100 cents per note relative to A_5 (440Hz)
    // tone_delay = 1/(2*1e-6*f) = 1/(2*1e-6*440*2^(cents_relative/1200))
    //   using uDelay(), 50% duty cycle, cents relative to A_4
    // tone_delay // cents, ideal frequency
    NOTE_C                            = 30578,  // -5700, 16.35159783 Hz, C_0
    NOTE_Cs                           = 28862,  // -5600, 17.32391444 Hz, Cs_0
    NOTE_Db                           = 28862,  // -5600, 17.32391444 Hz, Db_0
    NOTE_D                            = 27242,  // -5500, 18.35404799 Hz, D_0
    NOTE_Ds                           = 25713,  // -5400, 19.44543648 Hz, Ds_0
    NOTE_Eb                           = 25713,  // -5400, 19.44543648 Hz, Eb_0
    NOTE_E                            = 24270,  // -5300, 20.60172231 Hz, E_0
    NOTE_F                            = 22908,  // -5200, 21.82676446 Hz, F_0
    NOTE_Fs                           = 21622,  // -5100, 23.12465142 Hz, Fs_0
    NOTE_Gb                           = 21622,  // -5100, 23.12465142 Hz, Gb_0
    NOTE_G                            = 20408,  // -5000, 24.49971475 Hz, G_0
    NOTE_Gs                           = 19263,  // -4900, 25.9566436  Hz, Gs_0
    NOTE_Ab                           = 19263,  // -4900, 25.9565436  Hz, Ab_0
    NOTE_A                            = 18182,  // -4800, 27.5        Hz, A_0
    NOTE_As                           = 17161,  // -4700, 29.13523509 Hz, As_0
    NOTE_Bb                           = 17161,  // -4700, 29.13523509 Hz, Bb_0
    NOTE_B                            = 16198,  // -4600, 30.86770633 Hz, B_0
} buzzed_note_t;
typedef struct EasyPort_s {
	GPIO_TypeDef* port;
	uint16 pin;
} EasyPort_t;

#define DXL_MAXNUM_TXPARAM		150
#define DXL_MAXNUM_RXPARAM		60


#define BROADCAST_ID					254
#define INST_PING							1
#define INST_READ							2
#define INST_WRITE						3
#define INST_REG_WRITE				4
#define INST_ACTION						5
#define INST_RESET						6
#define INST_SYNC_WRITE				131
#define ID										2
#define LENGTH								3
#define INSTRUCTION						4
#define ERRBIT								4
#define PARAMETER							5


static uint8 gbInstructionPacket[DXL_MAXNUM_TXPARAM + 10];// = {0};
static uint8 gbStatusPacket[DXL_MAXNUM_RXPARAM + 10];// = {0};
static uint8 gbRxPacketLength;// = 0;
static uint8 gbRxGetLength;// = 0;

static volatile uint16 gbCommStatus = COMM_RXSUCCESS;
static volatile uint8 giBusUsing = 0;

#define TORQUE_OFF				254, 24, 0
#define DEFAULT_BAUD			57600
// USART Baudrates and whatnot
#define BAUD_DXL					1000000
#define BAUD_ZIG					DEFAULT_BAUD
#define BAUD_PC 					DEFAULT_BAUD
#define USART_DXL					0
#define USART_ZIG					1
#define USART_PCU					2

/**		Custom definitions for readability + legacy support		**/
#define word	uint32
#define byte	uint16
#define On		1
#define	Off		0


/************************************* Dynamixel bit reference definitions ********************************************/
// Will match a hex value followed by a text value and rewrite it as #define text hex
//	(\d+)x([:xdigit:]+)(\s+)(.*)
//	#define \4 \1x\2


#define DXL_MODEL_NUM_L				0x00
#define DXL_MODEL_NUM_H				0x01
#define DXL_FIRMWARE_VR				0x02
#define DXL_ID								0x03
#define DXL_BAUDRATE					0x04
#define DXL_DELAY_TIME				0x05
#define DXL_CWAngle_Limit_L		0x06
#define DXL_CWAngle_Limit_H		0x07
#define DXL_CCWAngle_Limit_L	0x08
#define DXL_CCWAngle_Limit_H	0x09

#define DXL_TEMP_LIMIT				0x0B
#define DXL_VOLT_LOW_LIMIT		0x0C
#define DXL_VOLT_HIGH_LIMIT		0x0D
#define DXL_TORQ_MAX_L				0x0E
#define DXL_TORQ_MAX_H				0x0F
#define DXL_STAT_RETURN_LVL		0x10
#define DXL_ALARM_LED					0x11
#define	DXL_ALARM_SHUTDOWN		0x12

#define DXL_TORQ_ENABLE				0x18
#define DXL_LED								0x19
#define DXL_CW_COMP_MARGIN		0x1A
#define DXL_CCW_COMP_MARGIN		0x1B
#define DXL_CW_COMP_SLOPE			0x1C
#define DXL_CCW COMP_SLOPE		0x1D
#define DXL_GOAL_POS_L				0x1E
#define DXL_GOAL_POS_H				0x1F
#define DXL_MOVING_SPEED_L		0x20
#define DXL_MOVING_SPEED_H		0x21
#define DXL_TORQ_LIMIT_L			0x22
#define DXL_TORQ_LIMIT_H			0x23
#define DXL_CURR_POS_L				0x24
#define DXL_CURR_POS_H				0x25
#define DXL_CURR_SPEED_L			0x26
#define DXL_CURR_SPEED_H			0x27
#define DXL_CURR_LOAD_L				0x28
#define DXL_CURR_LOAD_H				0x29
#define DXL_CURR_VOLT					0x2A
#define DXL_CURR_TEMP					0x2B
#define DXL_REGISTERED				0x2C

#define DXL_MOVING						0x2E
#define DXL_LOCKING						0x2F
#define DXL_PUNCH_L						0x30
#define DXL_PUNCH_H						0x31


/************************************* Component's ports and pin definitions ******************************************/
// Button/Mic PORT and PIN definitions
#define PORT_SW_UP                      GPIOC
#define PORT_SW_DOWN                    GPIOC
#define PORT_SW_RIGHT                   GPIOA
#define PORT_SW_LEFT                    GPIOA
#define PORT_SW_START                   GPIOB
#define PORT_MIC                        GPIOC
#define PIN_SW_UP                       GPIO_Pin_11
#define PIN_SW_DOWN                     GPIO_Pin_10
#define PIN_SW_RIGHT                    GPIO_Pin_14
#define PIN_SW_LEFT                     GPIO_Pin_15
#define PIN_SW_START                    GPIO_Pin_3
#define PIN_MIC                         GPIO_Pin_4

// LED PORT and PIN definitions
#define PORT_LED_POWER                  GPIOC
#define PORT_LED_MANAGE                 GPIOB
#define PORT_LED_PROGRAM                GPIOB
#define PORT_LED_PLAY                   GPIOB
#define PORT_LED_TXD                    GPIOC
#define PORT_LED_RXD                    GPIOC
#define PORT_LED_AUX                    GPIOB
#define PIN_LED_POWER                   GPIO_Pin_13
#define PIN_LED_MANAGE                  GPIO_Pin_13
#define PIN_LED_PROGRAM                 GPIO_Pin_14
#define PIN_LED_PLAY                    GPIO_Pin_15
#define PIN_LED_TXD                     GPIO_Pin_14
#define PIN_LED_RXD                     GPIO_Pin_15
#define PIN_LED_AUX                     GPIO_Pin_12

// Buzzer PORT and PIN definitions
#define PORT_BUZZER                     GPIOA
#define PIN_BUZZER                      GPIO_Pin_6

// ADC select PORT and PIN definitions
#define PORT_ADC_SELECT0                GPIOC
#define PORT_ADC_SELECT1                GPIOC
#define PIN_ADC_SELECT0                 GPIO_Pin_1
#define PIN_ADC_SELECT1                 GPIO_Pin_2
#define PIN_ADC0                        GPIO_Pin_0
#define PIN_ADC1                        GPIO_Pin_5
#define PIN_VDD_VOLT                    GPIO_Pin_3

// Ollo port PORT and PIN definitions
#define PORT_SIG_MOT1P                  GPIOA
#define PORT_SIG_MOT1M                  GPIOA
#define PORT_SIG_MOT2P                  GPIOA
#define PORT_SIG_MOT2M                  GPIOA
#define PORT_SIG_MOT3P                  GPIOC
#define PORT_SIG_MOT3M                  GPIOC
#define PORT_SIG_MOT4P                  GPIOC
#define PORT_SIG_MOT4M                  GPIOC
#define PORT_SIG_MOT5P                  GPIOA
#define PORT_SIG_MOT5M                  GPIOA
#define PORT_SIG_MOT6P                  GPIOB
#define PORT_SIG_MOT6M                  GPIOB
#define PIN_SIG_MOT1P                   GPIO_Pin_0
#define PIN_SIG_MOT1M                   GPIO_Pin_1
#define PIN_SIG_MOT2P                   GPIO_Pin_2
#define PIN_SIG_MOT2M                   GPIO_Pin_3
#define PIN_SIG_MOT3P                   GPIO_Pin_6
#define PIN_SIG_MOT3M                   GPIO_Pin_7
#define PIN_SIG_MOT4P                   GPIO_Pin_8
#define PIN_SIG_MOT4M                   GPIO_Pin_9
#define PIN_SIG_MOT5P                   GPIO_Pin_8
#define PIN_SIG_MOT5M                   GPIO_Pin_11
#define PIN_SIG_MOT6P                   GPIO_Pin_8
#define PIN_SIG_MOT6M                   GPIO_Pin_9

// Dynamixel
#define PORT_ENABLE_TXD                 GPIOB
#define PORT_ENABLE_RXD                 GPIOB
#define PORT_DXL_TXD                    GPIOB
#define PORT_DXL_RXD                    GPIOB
#define PIN_ENABLE_TXD                  GPIO_Pin_4
#define PIN_ENABLE_RXD                  GPIO_Pin_5
#define PIN_DXL_TXD                     GPIO_Pin_6
#define PIN_DXL_RXD                     GPIO_Pin_7

// Zigbee
#define PORT_ZIGBEE_TXD                 GPIOC
#define PORT_ZIGBEE_RXD                 GPIOD
#define PORT_ZIGBEE_RESET               GPIOA
#define PIN_ZIGBEE_TXD                  GPIO_Pin_12
#define PIN_ZIGBEE_RXD                  GPIO_Pin_2
#define PIN_ZIGBEE_RESET                GPIO_Pin_12

// Serial/PC_UART
#define PORT_PC_TXD                     GPIOB
#define PORT_PC_TXD                     GPIOB
#define PIN_PC_TXD                      GPIO_Pin_10
#define PIN_PC_RXD                      GPIO_Pin_11


/**
 0 (0X00)		Model Number(L)
 1 (0X01)		Model Number(H)
 2 (0X02)		Version of Firmware
 3 (0X03)		ID
 4 (0X04)		Baud Rate
 5 (0X05)		Return Delay Time
 6 (0X06)		CW Angle Limit(L)
 7 (0X07)		CW Angle Limit(H)
 8 (0X08)		CCW Angle Limit(L)
 9 (0X09)		CCW Angle Limit(H)

 11 (0X0B)		Highest Limit Temperature
 12 (0X0C)		Lowest Limit Voltage
 13 (0X0D)		Highest Limit Voltage
 14 (0X0E)		Max Torque(L)
 15 (0X0F)		Max Torque(H)
 16 (0X10)		Status Return Level
 17 (0X11)		Alarm LED
 18 (0X12)		Alarm Shutdown

 24 (0X18)		Torque Enable
 25 (0X19)		LED
 26 (0X1A)		CW Compliance Margin
 27 (0X1B)		CCW Compliance Margin
 28 (0X1C)		CW Compliance Slope
 29 (0X1D)		CCW Compliance Slope
 30 (0X1E)		Goal Position(L)
 31 (0X1F)		Goal Position(H)
 32 (0X20)		Moving Speed(L)
 33 (0X21)		Moving Speed(H)
 34 (0X22)		Torque Limit(L)
 35 (0X23)		Torque Limit(H)
 36 (0X24)		Present Position(L)
 37 (0X25)		Present Position(H)
 38 (0X26)		Present Speed(L)
 39 (0X27)		Present Speed(H)
 40 (0X28)		Present Load(L)
 41 (0X29)		Present Load(H)
 42 (0X2A)		Present Voltage
 43 (0X2B)		Present Temperature
 44 (0X2C)		Registered

 46 (0X2E)		Moving
 47 (0X2F)		Lock
 48 (0X30)		Punch(L)
 49 (0X31)		Punch(H)
**/

#endif