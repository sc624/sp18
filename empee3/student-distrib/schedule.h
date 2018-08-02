#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include "syscalls.h"

#define FIRST_SHELL    0
#define SECOND_SHELL   1
#define THIRD_SHELL    2

extern uint8_t cur_pid;         //var to keep track of current process
extern uint8_t term_counter;    //var to keep track of next terminal 

int32_t schedule();
#endif
