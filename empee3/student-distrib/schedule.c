#include "schedule.h"

uint8_t cur_pid = 0;    //0, 1, 2 reserved for terminal shells
uint8_t term_counter = 0;

int32_t schedule(){
    pcb_t* cur_task = get_pcb();

    /* Save ebp to use as reference */
    asm volatile("movl %%esp, %0" : "=r"(cur_task->kernel_stack));
    asm volatile("movl %%ebp, %0" : "=r"(cur_task->kernel_base));

    /* Determine next process */
    term_counter = (term_counter + 1) % MAX_TERMS;
    cur_pid = terminals[term_counter].active_pid;

    /* Launches shell 1 (terminal 1), shell 2 (terminal 2) if not launched  */
    if((pid_arr[SECOND_SHELL] == NOT_USED) || (pid_arr[THIRD_SHELL] == NOT_USED)) {
        do_execute((const uint8_t*) "shell");
    }

    /* Update paging */
    // Map 0x08000000 to 0x08400000 (128 to 132-MB) to (8MB + PARENT_PID * 4MB)
    page_directory[32] = (uint32_t) (FIRST_USER+(cur_pid)*FOUR_MB) | 0x87;
    flushTLB();

    /*
    If we use vidmap, and displayed terminal isn't the current terminal, write to backing page
    otherwise, we do normal video paging setup and display terminal
    */
    if (terminals[cur_task->on_term].vid_map_flag == 1){
        if (cur_task->on_term != curr_terminal){
            user_vidmem_page_table[0] = (uint32_t) (VIDEO_MEMORY + ((cur_task->on_term)+1) * (FOUR_KB)) | 0x7;
            flushTLB();
        }
        else{
            user_vidmem_page_table[0] = (uint32_t) VIDEO_MEMORY | 0x7;
            flushTLB();
        }
    }

    /* Set tss.esp0 to the bottom of new task's kernel stack */
    uint32_t next_kstack = (EIGHT_MB) - (cur_pid * EIGHT_KB);
    tss.esp0 = next_kstack;

    /* Restore ebp of next process */
    pcb_t* next_task = (pcb_t*) (next_kstack - EIGHT_KB);
    asm volatile("movl %0, %%esp" : :"r"(next_task->kernel_stack));
    asm volatile("popl %eax");
    asm volatile("movl %0, %%ebp" : :"r"(next_task->kernel_base));

    return 0;
}
