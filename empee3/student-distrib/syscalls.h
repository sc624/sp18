#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "x86_desc.h"
#include "types.h"
#include "lib.h"
#include "process.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "schedule.h"

/* Indices for fops table (jumptable) */
#define OPEN                  0
#define CLOSE                 1
#define READ                  2
#define WRITE                 3

#define EXCEP_RET           256
#define MAX_RUNNING_PROCESSES 6

// Syscall Wrapper functions
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler);
extern int32_t sigreturn(void);

// Syscall Implementations
extern int32_t do_halt (uint8_t status);
extern int32_t do_execute (const uint8_t* command);
extern int32_t do_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t do_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t do_open (const uint8_t* filename);
extern int32_t do_close (int32_t fd);
extern int32_t do_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t do_vidmap (uint8_t** screen_start);
extern int32_t do_set_handler (int32_t signum, void* handler);
extern int32_t do_sigreturn (void);

/* Helper functions*/

/* Initializes the PCB with default values */
extern int32_t init_pcb(uint32_t pid, pcb_t* pcb);
/* Returns the address of the current process' PCB */
extern pcb_t* get_pcb();
/* Sets up the stack for context switching (IRET) */
extern void context_setup(uint32_t entry_point, uint32_t user_stack, uint32_t pid);
/* Parses the sequence of words passed into execute as command and arguments */
extern int32_t parse_args(const uint8_t* str, uint8_t* cmd, uint8_t* args);

extern uint32_t pid_arr[MAX_RUNNING_PROCESSES];

#endif
