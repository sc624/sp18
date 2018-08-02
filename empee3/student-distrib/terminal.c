#include "terminal.h"

static int screen_x;
static int screen_y;
static char* video_mem = (char *)VIDEO;
static char terminal_buffer[BUFFER_SIZE];

extern int ENTER_FLAG;
extern int ENTER_FLAG_2, ENTER_FLAG_3;
extern uint8_t cur_pid;

int TERMINAL_READ;		//Read flag

/*
 * Terminal read
 *   DESCRIPTION: Read function for terimanal. When entered it will not exit until user presses
 * 				  enter on the keyboard. Writes terminal buffer to input buffer
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS:
 *   RETURN VALUE: Amount of bytes read
 *   SIDE EFFECTS: Clears terminal buffer on entry and writes to buffer passed in
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
	int i;
	cur_task = get_pcb();

	/* Checking for valid inputs */
	if (fd != 0 || fd > MAX_OPEN_FILES || buf == NULL || nbytes < 0) {
		return -1;
	}
	if (nbytes == 0){
		return 0;
	}

	TERMINAL_READ = 1;

	/* Waiting for user to press enter */
	LOOP:sti();
	while(!ENTER_FLAG && !ENTER_FLAG_2 && !ENTER_FLAG_3){}

	/* Begin critical section, clear interrupts*/
	while (cur_task->on_term != curr_terminal){
		cur_task = get_pcb();
	}

	cli();
	if (!ENTER_FLAG && !ENTER_FLAG_2 && !ENTER_FLAG_3){
		TERMINAL_READ = 1;
		goto LOOP;
	}

	/* Set enter flag to 0, write to buffer, and set read flag to 0 */
	ENTER_FLAG = 0;
	ENTER_FLAG_2 = 0;
	ENTER_FLAG_3 = 0;
	for (i = 0; (i < nbytes && i < BUFFER_SIZE); i++) {
		*((uint8_t*)buf + i) = terminals[curr_terminal].keyboard_buf[i];
	}
	TERMINAL_READ = 0;

	/* End critical section */
	sti();

	/* Return number of bytes read */
	if (nbytes < terminals[cur_task->on_term].term_buf_index) {
		return nbytes;
	}
	else {
		return terminals[cur_task->on_term].term_buf_index + 1;
	}
}



/*
 * terminal_write
 *   DESCRIPTION: This function writes the data from buf to the terminal.
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS:
 *   RETURN VALUE: none
 *   SIDE EFFECTS: returns bytes written to terminal
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
	/* Checking for valid inputs */
	if (fd != 1 || fd > MAX_OPEN_FILES || buf == NULL || nbytes < 0) {
		return -1;
	}
	if (nbytes == 0){
		return 0;
	}

	/* Begin critical section, clear interrupts and save flags */
	uint32_t FLAGS;
	cli_and_save(FLAGS);

	/* Check if current process is on the active terminal, if not write to back page */
	cur_task = get_pcb();
	if (cur_task->on_term != curr_terminal)	{
		first_page_table[184] = (uint32_t) (VIDEO_MEMORY + (cur_task->on_term+1) * (FOUR_KB)) | 0x3;
		flushTLB();
	}

	/* Writing data from buf to the terminal buffer */
	clear_buffer();

	if (nbytes <= BUFFER_SIZE){
    for (terminals[cur_task->on_term].term_buf_index = 0; (terminals[cur_task->on_term].term_buf_index < nbytes && terminals[cur_task->on_term].term_buf_index < BUFFER_SIZE); terminals[cur_task->on_term].term_buf_index++) {
				terminals[cur_task->on_term].keyboard_buf[terminals[cur_task->on_term].term_buf_index] = *((uint8_t*)buf + terminals[cur_task->on_term].term_buf_index);
		}
		terminal_puts(terminals[cur_task->on_term].term_buf_index);
	} else {
		int i;
		int j = 0;
		int y = 0;
		for (i = 0; i < nbytes; i++) {
				if (j < BUFFER_SIZE){
					terminals[cur_task->on_term].keyboard_buf[j] = *((uint8_t*)buf + i);
					j++;
				}
				else if (j == BUFFER_SIZE){
					terminal_puts(BUFFER_SIZE);
					j = 0;
					i--;
					y++;
				}
		}
		terminal_puts(nbytes - (y * BUFFER_SIZE));
	}

	/* Change page to point back to video memory */
	clear_buffer();
	if (cur_task->on_term != curr_terminal){
		first_page_table[184] = (uint32_t) VIDEO_MEMORY | 0x3;
		flushTLB();
	}

	/* End critical section, restore flags */
	sti();
	restore_flags(FLAGS);

	/* Return amount of bytes written */
	return nbytes;
}

/*
 * terminal_open
 *   DESCRIPTION: Clears terminal
 *   INPUTS: none
 *   OUTPUTS:
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Clears video memory and puts cursor on top left corner
 */
int32_t terminal_open(const uint8_t* filename) {
	clear_buffer();
	init_terminals();
	return 0;
}

/*
 * terminal_close
 *   DESCRIPTION: Clears terminal
 *   INPUTS: none
 *   OUTPUTS:
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Clears video memory and puts cursor on top left corner
 */
int32_t terminal_close(int32_t fd) {
	terminal_clear();
	return 0;
}

/*
 * init_terminals
 *   DESCRIPTION: set up paging for all 3 terminals and video details
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: can switch between all 3 terminals on startup
 */
void init_terminals(void)
{
	int i;
	//start first current terminal to first terminal
	curr_terminal = 0;
	/* setting up video paging for all 3 terminals */
	memcpy(&(term_vid_buf), (int8_t *)video_mem, NUM_ROWS * NUM_COLS * 2);
	first_page_table[184] = (uint32_t) (VIDEO_MEMORY + (curr_terminal+1) * (FOUR_KB)) | 0x3;
	flushTLB();
	memcpy((int8_t *)video_mem, &(term_vid_buf), NUM_ROWS * NUM_COLS * 2);

	/* write first page for video memory */
	first_page_table[184] = (uint32_t) VIDEO_MEMORY | 0x3;
	flushTLB();

	/* set up all terminal struct elements */
	terminals[curr_terminal].term_index = index;
	memcpy(&(terminals[curr_terminal].keyboard_buf), (int8_t *)terminal_buffer, BUFFER_SIZE);
	terminals[curr_terminal].term_buf_index = buf_index;
	terminals[curr_terminal].term_screen_x = screen_x;
	terminals[curr_terminal].term_screen_y = screen_y;
	terminals[curr_terminal].vid_map_flag = 0;

	/* initialize each video memory page for separate terminals */
	for (i = 1; i < MAX_TERMS; i++){
		first_page_table[184] = (uint32_t) (VIDEO_MEMORY + (i+1) * (FOUR_KB)) | 0x3;
		flushTLB();
		clear();
		terminals[i].term_index = 0;
		memset(&(terminals[i].keyboard_buf), 0, BUFFER_SIZE);
		terminals[i].term_buf_index = 0;
		terminals[i].term_screen_x = 0;
		terminals[i].term_screen_y = 0;
		terminals[i].vid_map_flag = 0;
	}
	first_page_table[184] = (uint32_t) VIDEO_MEMORY | 0x3;
	flushTLB();
}

/*
 * update_terminal
 *   DESCRIPTION: change video coordinates, terminal index, and update cursor
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS:
 */
void update_terminal(void)
{
	memcpy((int8_t *)terminal_buffer, &(terminals[curr_terminal].keyboard_buf), BUFFER_SIZE);
	buf_index = terminals[curr_terminal].term_buf_index;
	screen_x = terminals[curr_terminal].term_screen_x;
	screen_y = terminals[curr_terminal].term_screen_y;
	index = terminals[curr_terminal].term_index;
	update_cursor();
}


/*
 * switch_terminal
 *   DESCRIPTION: Save/restore video memory, Switch video paging, Switch Input keyboard buffer, Update visible video coordinates
 *   INPUTS: term_dest - terminal that we want to switch to
 *   OUTPUTS: None
 *	 RETURN VALUE: None
 *   SIDE EFFECTS: switches terminal thats displayed
 */
void switch_terminal(const int term_dest)
{
	cli();
	cur_task = get_pcb();

	/* Saving back information */
	memcpy(&(term_vid_buf), (int8_t *)video_mem, NUM_ROWS * NUM_COLS * 2);
	first_page_table[184] = (uint32_t) (VIDEO_MEMORY + (curr_terminal+1) * (FOUR_KB)) | 0x3;
	flushTLB();
	memcpy((int8_t *)video_mem, &(term_vid_buf), NUM_ROWS * NUM_COLS * 2);

	/* Saving keyboard input buffer */
	memcpy(&(terminals[curr_terminal].keyboard_buf), (int8_t *)terminal_buffer, BUFFER_SIZE);
	terminals[curr_terminal].term_index = index;
	memcpy(&(terminals[curr_terminal].keyboard_buf), (int8_t *)terminal_buffer, BUFFER_SIZE);

	/* Update running video coordinates */
	terminals[curr_terminal].term_buf_index = buf_index;
	terminals[curr_terminal].term_screen_x = screen_x;
	terminals[curr_terminal].term_screen_y = screen_y;


	/* Restoring desired terminal information */
	if (terminals[term_dest].vid_map_flag == 0)
	{
		if (terminals[curr_terminal].vid_map_flag == 1)
		{
			user_vidmem_page_table[0] = (uint32_t) (VIDEO_MEMORY + (curr_terminal+1) * (FOUR_KB)) | 0x7;
		}
	}
	else{
		user_vidmem_page_table[0] = (uint32_t) VIDEO_MEMORY | 0x7;
	}

	/* Setting up video paging to switch to desired terminal video address */
	first_page_table[184] = (uint32_t) (VIDEO_MEMORY + (term_dest+1) * (FOUR_KB)) | 0x3;
	flushTLB();
	memcpy(&(term_vid_buf), (int8_t *)video_mem, NUM_ROWS * NUM_COLS * 2);
	first_page_table[184] = (uint32_t) VIDEO_MEMORY | 0x3;
	flushTLB();

	/* Restoring backing information */
	memcpy((int8_t *)video_mem, &(term_vid_buf), NUM_ROWS * NUM_COLS * 2);
	memcpy((int8_t *)terminal_buffer, &(terminals[term_dest].keyboard_buf), BUFFER_SIZE);

	/* updating video coordinates and terminal info */
	index = terminals[term_dest].term_index;
	buf_index = terminals[term_dest].term_buf_index;
	screen_x = terminals[term_dest].term_screen_x;
	screen_y = terminals[term_dest].term_screen_y;
	curr_terminal = term_dest;
	index = terminals[term_dest].term_index;

	update_cursor();
	sti();

}

/*
 * terminal_clear
 *   DESCRIPTION: Clears video memory and puts cursor on top left
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void terminal_clear(void) {
	clear();
	index = 0;
	screen_x = 0;
	screen_y = 0;
	terminals[curr_terminal].term_index = 0;
	terminals[curr_terminal].term_screen_x = 0;
	terminals[curr_terminal].term_screen_y = 0;
	update_cursor();
}

/*
 * clear_buffer
 *   DESCRIPTION: Clears buffer of size 128
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void clear_buffer(void) {
	cur_task = get_pcb();
	int i;
	buf_index = 0;
	terminals[cur_task->on_term].term_buf_index = 0;
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		terminal_buffer[i] = 0;
		terminals[cur_task->on_term].keyboard_buf[i] = 0;
	}
}

/*
 * scroll_up
 *   DESCRIPTION: Scrolls the terminal up in whichever active program is scheduled
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Manipulates video memory
 */
void scroll_up(void) {
	cur_task = get_pcb();
	int x;
	int y;
	int dy;
	uint8_t f;
	/* Iterating through video memory */
	for (y = 1; y < NUM_ROWS; y++) {
		for (x = 0; x < NUM_COLS; x++) {
			dy = y;
			dy--;
			f = *(uint8_t *)(video_mem + ((NUM_COLS * y + x) << 1));
			*(uint8_t *)(video_mem + ((NUM_COLS * dy + x) << 1)) = f;
			*(uint8_t *)(video_mem + ((NUM_COLS * dy + x) << 1) + 1) = ATTRIB;
		}
	}

	/* Clearing the last row */
	for (x = 0; x < 80; x++) {
		*(uint8_t *)(video_mem + ((80 * 24 + x) << 1)) = ' ';
		*(uint8_t *)(video_mem + ((80 * 24 + x) << 1) + 1) = ATTRIB;
	}

	/* Put cursor at beginning of the last row */
	terminals[cur_task->on_term].term_index = 1920;
	terminals[cur_task->on_term].term_screen_x = 0;
	terminals[cur_task->on_term].term_screen_y = 24;
	if (cur_task->on_term == curr_terminal)
	{
		update_terminal();
	}
}


/*
 * scroll_up_curr
 *   DESCRIPTION: scrolls up in the current terminal you are in
 *   INPUTS: none
 *   OUTPUTS:
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Clears video memory and puts cursor on top left corner
 */
void scroll_up_curr(void) {
	int x;
	int y;
	int dy;
	uint8_t f;
	/* Iterating through video memory */
	for (y = 1; y < NUM_ROWS; y++) {
		for (x = 0; x < NUM_COLS; x++) {
			dy = y;
			dy--;
			f = *(uint8_t *)(video_mem + ((NUM_COLS * y + x) << 1));
			*(uint8_t *)(video_mem + ((NUM_COLS * dy + x) << 1)) = f;
			*(uint8_t *)(video_mem + ((NUM_COLS * dy + x) << 1) + 1) = ATTRIB;
		}
	}

	/* Clearing the last row */
	for (x = 0; x < 80; x++) {
		*(uint8_t *)(video_mem + ((80 * 24 + x) << 1)) = ' ';
		*(uint8_t *)(video_mem + ((80 * 24 + x) << 1) + 1) = ATTRIB;
	}

	/* Put cursor at beginning of the last row */
	terminals[curr_terminal].term_index = 1920;
	terminals[curr_terminal].term_screen_x = 0;
	terminals[curr_terminal].term_screen_y = 24;
	update_terminal();
}

/*
 * terminal_putc
 *   DESCRIPTION: Prints chars to the terminal
 *   INPUTS: c, d
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints to terminal
 * 	 referenced from lib.c
 */
void terminal_putc(char c, int d){
	/* Checking if our buffer is full or if we need to scroll*/
	if (terminals[curr_terminal].term_buf_index < BUFFER_SIZE-1 && terminals[curr_terminal].term_index < 2000) {
		terminals[curr_terminal].keyboard_buf[terminals[curr_terminal].term_buf_index] = c;

		/* Checking our delete flag */
		if (!d){
			terminals[curr_terminal].term_index++;
			terminals[curr_terminal].term_buf_index++;
		}

		/* Handling char value appropriately, printing to terminal */
		if(c == '\n' || c == '\r') {
			terminals[curr_terminal].term_screen_y++;
			terminals[curr_terminal].term_screen_x = 0;
		} else if (terminals[curr_terminal].term_index % 80 == 0) {
			*(uint8_t *)(video_mem + ((NUM_COLS * terminals[curr_terminal].term_screen_y + terminals[curr_terminal].term_screen_x) << 1)) = c;
			*(uint8_t *)(video_mem + ((NUM_COLS * terminals[curr_terminal].term_screen_y + terminals[curr_terminal].term_screen_x) << 1) + 1) = ATTRIB;
			terminals[curr_terminal].term_screen_y++;
			terminals[curr_terminal].term_screen_x = 0;
		} else {
			*(uint8_t *)(video_mem + ((NUM_COLS * terminals[curr_terminal].term_screen_y + terminals[curr_terminal].term_screen_x) << 1)) = c;
			*(uint8_t *)(video_mem + ((NUM_COLS * terminals[curr_terminal].term_screen_y + terminals[curr_terminal].term_screen_x) << 1) + 1) = ATTRIB;
			terminals[curr_terminal].term_screen_x++;
			terminals[curr_terminal].term_screen_x %= NUM_COLS;
			terminals[curr_terminal].term_screen_y = (terminals[curr_terminal].term_screen_y + (terminals[curr_terminal].term_screen_x / NUM_COLS)) % NUM_ROWS;
		}
		update_terminal();
	}
	/* Checking if we need to scroll */
	if (terminals[curr_terminal].term_index >= 2000){
		scroll_up_curr();
	}
}


/*
 * terminal_puts
 *   DESCRIPTION: Prints buffer to the terminal
 *   INPUTS: nbytes - the number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes video memory
 *   Referenced from lib.c
 */
void terminal_puts(int32_t nbytes) {
	int i;
	cur_task = get_pcb();
	/* Iterate through keyboard buf */
	for (i = 0; i < nbytes; i++)
	{
		/* Check for null termination */
		if (terminals[cur_task->on_term].keyboard_buf[i] != '\0'){
			/* Check if we need to make a new line */
			if(terminals[cur_task->on_term].keyboard_buf[i] == '\n' || terminals[cur_task->on_term].keyboard_buf[i] == '\r' || terminals[cur_task->on_term].term_screen_x == 80) {
				/* Case for if we are not at the end of the screen */
				if (terminals[cur_task->on_term].term_screen_x != 80)
				{
					terminals[cur_task->on_term].term_screen_y++;
					terminals[cur_task->on_term].term_screen_x = 0;
					terminals[cur_task->on_term].term_index = (terminals[cur_task->on_term].term_screen_y * NUM_COLS) + terminals[cur_task->on_term].term_screen_x;
					if (terminals[cur_task->on_term].term_index >= 2000) {
						scroll_up();
					}
				/* Case for if we are on the end of the screen and see a '\n' */
				} else if (terminals[cur_task->on_term].keyboard_buf[i] != '\n') {
					terminals[cur_task->on_term].term_screen_x = 0;
					terminals[cur_task->on_term].term_screen_y++;
					terminals[cur_task->on_term].term_index = (terminals[cur_task->on_term].term_screen_y * NUM_COLS) + terminals[cur_task->on_term].term_screen_x;
					if (terminals[cur_task->on_term].term_index >= 2000) {
						scroll_up();
					}
					*(uint8_t *)(video_mem + ((NUM_COLS * terminals[cur_task->on_term].term_screen_y + terminals[cur_task->on_term].term_screen_x) << 1)) = terminals[cur_task->on_term].keyboard_buf[i];
					*(uint8_t *)(video_mem + ((NUM_COLS * terminals[cur_task->on_term].term_screen_y + terminals[cur_task->on_term].term_screen_x) << 1) + 1) = ATTRIB;
					terminals[cur_task->on_term].term_screen_x++;
					terminals[cur_task->on_term].term_screen_y = (terminals[cur_task->on_term].term_screen_y + (terminals[cur_task->on_term].term_screen_x / NUM_COLS)) % NUM_ROWS;
				/* Case if we are at the end of the screen and don't see a '\n'*/
				} else {
					terminals[cur_task->on_term].term_screen_x = 0;
					terminals[cur_task->on_term].term_screen_y++;
					terminals[cur_task->on_term].term_index = (terminals[cur_task->on_term].term_screen_y * NUM_COLS) + terminals[cur_task->on_term].term_screen_x;
					if (terminals[cur_task->on_term].term_index >= 2000) {
						scroll_up();
					}
				}
			/* If not a new line then print char to video memory normally and update terminal struct member variables */
			} else {
				*(uint8_t *)(video_mem + ((NUM_COLS * terminals[cur_task->on_term].term_screen_y + terminals[cur_task->on_term].term_screen_x) << 1)) = terminals[cur_task->on_term].keyboard_buf[i];
				*(uint8_t *)(video_mem + ((NUM_COLS * terminals[cur_task->on_term].term_screen_y + terminals[cur_task->on_term].term_screen_x) << 1) + 1) = ATTRIB;
				terminals[cur_task->on_term].term_screen_x++;
				terminals[cur_task->on_term].term_screen_y = (terminals[cur_task->on_term].term_screen_y == NUM_ROWS - 1) ? terminals[cur_task->on_term].term_screen_y : (terminals[cur_task->on_term].term_screen_y + (terminals[cur_task->on_term].term_screen_x / NUM_COLS)) % NUM_ROWS;
			}
			terminals[cur_task->on_term].term_index = (terminals[cur_task->on_term].term_screen_y * NUM_COLS) + terminals[cur_task->on_term].term_screen_x;
			if (cur_task->on_term == curr_terminal)
			{
				update_terminal();
			}
		}
	}
}


/*
 * terminal_newline
 *   DESCRIPTION: Creates a new line in the terminal and clears the buffer. Scrolls if it needs to.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void terminal_newline(void) {
	cur_task = get_pcb();
	uint32_t flags;
	/* Set enter flag to 1 and append a new line to buffer */
		cli_and_save(flags);
		switch(curr_terminal){
			case 0:
				ENTER_FLAG = 1;
				ENTER_FLAG_2 = 0;
				ENTER_FLAG_3 = 0;
				break;
			case 1:
				ENTER_FLAG_2 = 1;
				ENTER_FLAG_3 = 0;
				ENTER_FLAG = 0;
				break;
			case 2:
				ENTER_FLAG_3 = 1;
				ENTER_FLAG_2 = 0;
				ENTER_FLAG = 0;
				break;
			default:
				break;
		}
		terminals[curr_terminal].keyboard_buf[terminals[curr_terminal].term_buf_index] = '\n';

		/* Check if we need to scroll, if not make a new line */
		if (terminals[curr_terminal].term_index >= 1920) {
			scroll_up_curr();
		}
		else {
			terminals[curr_terminal].term_screen_x = 0;
			terminals[curr_terminal].term_screen_y++;
			terminals[curr_terminal].term_index = (terminals[curr_terminal].term_screen_y * NUM_COLS) + terminals[curr_terminal].term_screen_x;
			update_terminal();
		}
		restore_flags(flags);
		sti();
}

/*
 * cursor_backspace
 *   DESCRIPTION: moves cursor back one space and deletes character when backspace is pressed
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes video memory
 */
void cursor_backspace(void) {
	int BACK_FLAG;
	/* Checking to see if we are in the beginning of a row or at index 0 */
	if (terminals[curr_terminal].term_index % 80 == 0 && terminals[curr_terminal].term_index != 0) {
		/* Set backspace flag to one and shift coordinates*/
		BACK_FLAG = 1;
		terminals[curr_terminal].term_index--;
		terminals[curr_terminal].term_screen_y--;
		terminals[curr_terminal].term_screen_x = NUM_COLS - 1;

		/* call terminal putc with delete flag */
		terminal_putc('\0', BACK_FLAG);
		terminals[curr_terminal].term_screen_x = NUM_COLS - 1;
		terminals[curr_terminal].term_buf_index--;

		/* Update cursor */
		update_terminal();
		BACK_FLAG = 0;
	} else if (terminals[curr_terminal].term_buf_index > 0 && terminals[curr_terminal].term_buf_index < BUFFER_SIZE - 1 && terminals[curr_terminal].term_index != 0) {
		/* Set backspace flag to one and shift coordinates*/
		BACK_FLAG = 1;
		terminals[curr_terminal].term_screen_x--;

		/* call terminal putc with delete flag */
		terminal_putc('\0', BACK_FLAG);
		terminals[curr_terminal].term_screen_x--;
		terminals[curr_terminal].term_index--;
		terminals[curr_terminal].term_buf_index--;

		/* Update cursor */
		update_terminal();
		BACK_FLAG = 0;
	} else if (terminals[curr_terminal].term_buf_index == BUFFER_SIZE - 1) {
		/* Set backspace flag to one and shift coordinates*/
		BACK_FLAG = 1;
		terminals[curr_terminal].term_screen_x--;
		terminals[curr_terminal].term_index--;
		terminals[curr_terminal].term_buf_index--;

		/* call terminal putc with delete flag */
		terminal_putc('\0', BACK_FLAG);
		terminals[curr_terminal].term_screen_x--;

		/* Update cursor */
		update_terminal();
		BACK_FLAG = 0;
	}
}


/*
 * update_cursor
 *   DESCRIPTION: updates cursor in the terminal according to the value of index
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes video memory
 */
void update_cursor(void)
{
	/* Writing to cursor ports */
    outb(CURSOR_HIGH, CRTC_CURSOR_H);
    outb((uint8_t)(index), CRTC_CURSOR_L);

    outb(CURSOR_LOW, CRTC_CURSOR_H);
    outb((uint8_t)((index >> 8) & 0xFF), CRTC_CURSOR_L);
}


/*
 * lib_putc
 *   DESCRIPTION: displays character on screen
 *   INPUTS: character
 *   OUTPUTS: printed character
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: Clears video memory and puts cursor on top left corner
 */
void lib_putc(uint8_t c) {
    if(c == '\n' || c == '\r') {
        screen_y++;
        screen_x = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;
        screen_x %= NUM_COLS;
        screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
    }
	index = (screen_y * NUM_COLS) + screen_x;
	update_cursor();
}

/*
 * lib_puts
 *   DESCRIPTION: uses lib_putc to print string
 *   INPUTS: string
 *   OUTPUTS: string to print
 *   RETURN VALUE: size of string
 *   SIDE EFFECTS: None
 */
int32_t lib_puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        lib_putc(s[index]);
        index++;
    }
    return index;
}
