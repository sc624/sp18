//https://wiki.osdev.org/Keyboard//
#include "keyboard.h"

static int CAPS_FLAG;
static int SHIFT_L_FLAG;
static int SHIFT_R_FLAG;
static int CTRL_FLAG;
static int ALT_FLAG;

int ENTER_FLAG, ENTER_FLAG_2, ENTER_FLAG_3;

// Data for scan code 1
static const char scancode[2][SIZE] =
{{ 0,  27, '1', '2', '3', '4', '5', '6', '7', '8',    /* 9 */
 '9', '0', '-', '=', '\b',    /* Backspace */
 '\t',            /* Tab */
 'q', 'w', 'e', 'r',    /* 19 */
 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',    /* Enter key */
   0,            /* 29   - Control */
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    /* 39 */
'\'', '`',   0,        /* Left shift */
'\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
 'm', ',', '.', '/', 0},

/* Upper Case Array */

 { 0,  27, '!', '@', '#', '$', '%', '^', '&', '*',    /* 9 */
 '(', ')', '_', '+', '\b',    /* Backspace */
 '\t',            /* Tab */
 'Q', 'W', 'E', 'R',    /* 19 */
 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',    /* Enter key */
   0,            /* 29   - Control */
 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',    /* 39 */
'"', '~',   0,        /* Left shift */
'|', 'Z', 'X', 'C', 'V', 'B', 'N',            /* 49 */
 'M', '<', '>', '?', 0}};


/*
 * keyboard_init
 *   DESCRIPTION: this function initializes the keyboard by
 *                enabling the PS2 IRQ line and then sending
 *                a byte to the port to enable scan code 1
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: allows interrupts from keyboard to be recieved
 */
void keyboard_init()
{
	cli();                     // setting up critical section
    CAPS_FLAG = 0;
    SHIFT_L_FLAG = 0;
    CTRL_FLAG = 0;
    ALT_FLAG = 0;
	enable_irq(PS2_IRQ_LINE);  // enabling the irq line
    outb(PS2_SCAN_PORT, 1);    // setting to scan code
	sti();
}

/*
 * getScancode
 *   DESCRIPTION: this function reads from the PS2 data port and
 *                and returns its value
 *   INPUTS: none
 *   OUTPUTS: char value of port
 *   RETURN VALUE: none
 *   SIDE EFFECTS: returns char
 */
char getScancode()
{
    char c = 0;
    c = inb(PS2_PORT);      // reading from ps2 data port
    return c;
}

/*
 * keyboard_handler
 *   DESCRIPTION: this function is called everytime we receieve
 *                an interrupt from the keyboard. The interrupt
 *                service routine reads from the scancode and then
 *                determines if the interrupt was a key press or release
 *                and prints the key press to the screen.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints to the screen
 */
extern void keyboard_handler()
{
    char d;
    disable_irq(PS2_IRQ_LINE);  // begin critical section
    char c = getScancode();

    switch((int)c){
        case CAPSLOCK_PRESS:
            CAPS_FLAG ^= 1;
            break;
        case LEFT_SHIFT_PRESS:
            SHIFT_L_FLAG ^= 1;
            break;
        case LEFT_SHIFT_RELEASE:
            SHIFT_L_FLAG ^= 1;
            break;
        case RIGHT_SHIFT_PRESS:
            SHIFT_R_FLAG ^= 1;
            break;
        case RIGHT_SHIFT_RELEASE:
            SHIFT_R_FLAG ^= 1;
            break;
        case LEFT_CONTROL_PRESS:
            CTRL_FLAG ^= 1;
            break;
        case LEFT_CONTROL_RELEASE:
            CTRL_FLAG ^= 1;
            break;
        case RIGHT_ALT_PRESS:
            ALT_FLAG = 1;
            break;
        case RIGHT_ALT_RELEASE:
            ALT_FLAG = 0;
            break;
        case ENTER_PRESSED:
            ENTER_FLAG = 1;
            break;
        default:
        break;
    }
    if (c == ENTER_PRESSED)
    {
        terminal_newline();
    }
    else if (c == L_PRESS && CTRL_FLAG)
    {
        terminal_clear();
    }
    else if (c == BACKSPACE_PRESS)
    {
        cursor_backspace();
    }
    else if(c == SPACE_PRESSED){
        d = ' ';
        terminal_putc(d,0);
    }
    else if (c == F1_PRESS && ALT_FLAG)
    {
        switch_terminal(TERMINAL_1);
    }
    else if (c == F2_PRESS && ALT_FLAG)
    {
        switch_terminal(TERMINAL_2);
    }
    else if (c == F3_PRESS && ALT_FLAG)
    {
        switch_terminal(TERMINAL_3);
    }
    else if(c >= 0 && (!CAPS_FLAG && !SHIFT_L_FLAG && !SHIFT_R_FLAG && !ENTER_FLAG && !ALT_FLAG) && (c != CAPSLOCK_PRESS && c != LEFT_SHIFT_PRESS && c != RIGHT_SHIFT_PRESS && c != LEFT_CONTROL_PRESS && c != RIGHT_ALT_PRESS && c != ENTER_PRESSED && c != F1_PRESS && c != F2_PRESS && c != F3_PRESS))
    {
        d = scancode[0][(int)c];
        terminal_putc(d, 0);
    }
    else if(c >= 0 && (CAPS_FLAG || SHIFT_L_FLAG || SHIFT_R_FLAG) && (c != CAPSLOCK_PRESS && c != LEFT_SHIFT_PRESS && c != RIGHT_SHIFT_PRESS && c != LEFT_CONTROL_PRESS && c != RIGHT_ALT_PRESS && c != ENTER_PRESSED && c != F1_PRESS && c != F2_PRESS && c != F3_PRESS))
    {
        d = scancode[1][(int)c];
        terminal_putc(d, 0);
    }
    send_eoi(PS2_IRQ_LINE);     // end of interrupt
    enable_irq(PS2_IRQ_LINE);   // end critical section
}
