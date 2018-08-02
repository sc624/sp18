#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define SIZE 			128
#define PS2_PORT 		0x60
#define PS2_SCAN_PORT	0x70
#define PS2_IRQ_LINE	1

#define CAPSLOCK_PRESS			0x3A
#define LEFT_SHIFT_PRESS		0x2A
#define LEFT_SHIFT_RELEASE		-86
#define RIGHT_SHIFT_PRESS		0x36
#define RIGHT_SHIFT_RELEASE		-74
#define BACKSPACE_PRESS         0x0E
#define BACKSPACE_RELEASE       0x8E
#define LEFT_CONTROL_PRESS      0x1D
#define LEFT_CONTROL_RELEASE    -99
#define RIGHT_ALT_PRESS         0x38
#define RIGHT_ALT_RELEASE       -72
#define ENTER_PRESSED           0x1C
#define ENTER_RELEASED          -100
#define SPACE_PRESSED           0x39

#define F1_PRESS                0x3B
#define F1_RELEASE              -69
#define F2_PRESS                0x3C
#define F2_RELEASE              -68
#define F3_PRESS                0x3D
#define F3_RELEASE              -67


extern int ENTER_FLAG;

#define L_PRESS                 0x26

#include "lib.h"
#include "i8259.h"
#include "terminal.h"

/*
 * keyboard_init
 *   DESCRIPTION: this function initializes the keyboard by
 *				  enabling the PS2 IRQ line and then sending
 *				  a byte to the port to enable scan code 1
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: allows interrupts from keyboard to be recieved
 */
void keyboard_init();

/*
 * getScancode
 *   DESCRIPTION: this function reads from the PS2 data port and
 *    	          and returns its value
 *   INPUTS: none
 *   OUTPUTS: char value of port
 *   RETURN VALUE: none
 *   SIDE EFFECTS: returns char
 */
char getScancode();

/*
 * keyboard_handler
 *   DESCRIPTION: this function is called everytime we receieve
 *				  an interrupt from the keyboard. The interrupt
 *				  service routine reads from the scancode and then
 *				  determines if the interrupt was a key press or release
 * 				  and prints the key press to the screen.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints to the screen
 */
extern void keyboard_handler();

#endif
