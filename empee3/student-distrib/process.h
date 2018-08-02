#ifndef PROCESS_H
#define PROCESS_H

#include "filesys.h"

#define MAX_OPEN_FILES          8
#define MAX_RUNNING_PROCESSES   6
#define USED                    1
#define NOT_USED                0

#define BUFFER_SIZE   128

/* Taken from lecture notes */
typedef struct file_object {
    int32_t *fops;
    int32_t inode_num;
    int32_t fpos;
    int32_t flags;
} file_t;

extern file_t fd_arr[MAX_OPEN_FILES];

/*
 * pid - process id of the current process pcb
 * on_term - current terminal that process is on
 * fd_arr[MAX_OPEN_FILES] - fd array for each process
 * kernel_stack - esp of pcb
 * kernel_base - ebp of pcb
 * parent - parent process
 * buffCopyArg - arguments of command
 */
typedef struct pcb {
    uint32_t pid;
    uint32_t on_term;
    file_t fd_arr[MAX_OPEN_FILES];
    uint32_t kernel_stack;
    uint32_t kernel_base;
    struct pcb* parent;
    uint8_t buffCopyArg[BUFFER_SIZE];
} pcb_t;

#endif
