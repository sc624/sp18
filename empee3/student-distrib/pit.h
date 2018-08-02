#ifndef  _PIT_H
#define  _PIT_H

#include "lib.h"
#include "i8259.h"
#include "lib.h"
#include "schedule.h"

#define CHANNEL_0_RW_PIT    0x40
#define CHANNEL_1_RW_PIT    0x41
#define CHANNEL_2_RW_PIT    0x42
#define COMMAND_RGSTR_PIT   0x43
#define IRQ_PIT             0
#define SHIFT_BIT           8

#define MAX_FREQ_PIT        1193182
#define MASK_FREQ           0xFF
#define MODE_3              0x36
#define MODE_2              0x34
#define COUNTER_DIVIDER     40

extern void pit_init();
extern void pit_handler();
#endif
