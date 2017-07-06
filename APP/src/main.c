
#include "API.h"

void blinky(uint8 level);


int main(void) {
	SysInit();
	int i = 0;
	led_init();
	dxl_init();
	mDelay(500);
	for (i = 0; i < 18; i++)
		ping_dxl(i);
	mDelay(1000);
	PrintLine("\nManipulating DXL-03:\nCurrent Position:");
	Printu16h(dxl_read_word(3, DXL_CURR_POS_L));
	PrintLine("Turning off torque... ");
	setDXLTorqueState(11, Off);
	PrintString("Done");
	PrintLine("Setting DXL-03's Position to 640 (3/4 full)...\n");
	setDXLPosition(3, 640);
	printStatus();
	PrintLine("Position Set. Turning torque on... ");
	setDXLTorqueState(3, On);
	PrintString("Done.\n");
			//dxl_get_result() == DXL_TXSUCCESS?PrintString("Success"):PrintString("Failed");
	mDelay(50);
	end();
	return 0;
}

void blinky(uint8 level) {
	// Turn them all off first.
	SetLED(MANAGE, 0);
	SetLED(TXD, 0);
	SetLED(PROGRAM, 0);
	SetLED(RXD, 0);
	SetLED(PLAY, 0);
	SetLED(AUX, 0);
	switch (level) {	// Turn them on.
		case 3: SetLED(MANAGE, 1);	SetLED(TXD, 1); break;
		case 2: SetLED(PROGRAM, 1);	SetLED(RXD, 1); break;
		case 1: SetLED(PLAY, 1);		SetLED(AUX, 1); break;
		default: return;
	}
	mDelay(100);
	switch (level) {	// Turn them on.
		case 3: SetLED(MANAGE, 0);	SetLED(TXD, 0); break;
		case 2: SetLED(PROGRAM, 0);	SetLED(RXD, 0); break;
		case 1: SetLED(PLAY, 0);		SetLED(AUX, 0); break;
		default: return;
	}
	mDelay(100);
}
