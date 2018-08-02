#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "types.h"
#include "schedule.h"
#include "process.h"

/* Lib.c definitions */
#define VIDEO               0xB8000
#define NUM_COLS            80
#define NUM_ROWS            25
#define ATTRIB              0x7

#define BUFFER_SIZE 		128
#define CRTC_CURSOR_H       0x3D4
#define CRTC_CURSOR_L       0x3D5
#define CURSOR_START		0xA
#define CURSOR_END			0xB
#define CURSOR_HIGH      	0xF
#define CURSOR_LOW      	0xE

#define MAX_TERMS           3

#define TERMINAL_1         0
#define TERMINAL_2         1
#define TERMINAL_3         2

#define MAX_OPEN_FILES      8

typedef struct terminal{
    volatile uint16_t term_index;
    char keyboard_buf[BUFFER_SIZE];
    volatile uint16_t term_buf_index;
    int term_screen_x;
    int term_screen_y;
    uint8_t active_pid;
    uint8_t vid_map_flag;
}terminal_t;

uint8_t term_vid_buf[NUM_ROWS * NUM_COLS * 2];
uint8_t curr_terminal, prev_terminal;
pcb_t* cur_task;

terminal_t terminals[MAX_TERMS];

volatile static uint16_t index;
volatile static uint16_t buf_index;

void init_terminals(void);
void switch_terminal(const int term_dest);
void copy_test(void);
void copy_test2(void);

/* Terminal driver functions */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);

/* Terminal helper functions */
void terminal_clear(void);
void scroll_up(void);
void update_cursor(void);
void terminal_puts(int32_t nbytes);
void terminal_putc(char c, int d);
void cursor_backspace(void);
void terminal_newline(void);
void clear_buffer(void);
void update_terminal(void);
void scroll_up_curr(void);

void lib_putc(uint8_t c);
int32_t lib_puts(int8_t* s);
#endif
