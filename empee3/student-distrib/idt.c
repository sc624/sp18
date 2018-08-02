/*
IDT.C contains:
exception wrapper declarations
exception handlers
sys call handlers
IDT initialization & fill (for all 3 types of interrupts)
https://github.com/knusbaum/kernel/blob/master/idt.c
*/


#include "idt.h"

static void init_idt();
static void set_table(int i);

extern int isr0_wrapper;
extern int isr1_wrapper;
extern int isr2_wrapper;
extern int isr3_wrapper;
extern int isr4_wrapper;
extern int isr5_wrapper;
extern int isr6_wrapper;
extern int isr7_wrapper;
extern int isr8_wrapper;
extern int isr9_wrapper;
extern int isr10_wrapper;
extern int isr11_wrapper;
extern int isr12_wrapper;
extern int isr13_wrapper;
extern int isr14_wrapper;
extern int isr15_wrapper;
extern int isr16_wrapper;
extern int isr17_wrapper;
extern int isr18_wrapper;

extern int RTC_wrapper();
extern int KB_wrapper();
extern int PIT_wrapper();
extern int syscall_wrapper();

static char msg[BUFFER_SIZE];

/*
* init_descriptor_tables
*   DESCRIPTION: initializes idt
*   INPUTS:
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS:
*/
void init_descriptor_tables(){
    init_idt();
}

//exception handlers
/*
* isr0 - isr18
*   DESCRIPTION: prints out exception statement then halt loops
*   INPUTS:
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: halt loop
*/
void isr0 (){
    strcpy((int8_t*) msg, (const int8_t*) "Divide-By-Zero Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr1 (){
    strcpy((int8_t*) msg, (const int8_t*) "Debug Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr2 (){
    strcpy((int8_t*) msg, (const int8_t*) "Non-Maskable Interrupt (NMI) Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr3 (){
    strcpy((int8_t*) msg, (const int8_t*) "Breakpoint (INT 3) Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr4 (){
    strcpy((int8_t*) msg, (const int8_t*) "Overflow Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr5 (){
    strcpy((int8_t*) msg, (const int8_t*) "Bound Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr6 (){
    strcpy((int8_t*) msg, (const int8_t*) "Invalid Opcode Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr7 (){
    strcpy((int8_t*) msg, (const int8_t*) "FPU Not Available Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr8 (){
    strcpy((int8_t*) msg, (const int8_t*) "Double Fault\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr9 (){
    strcpy((int8_t*) msg, (const int8_t*) "Coprocessor Segment Overrun Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr10 (){
    strcpy((int8_t*) msg, (const int8_t*) "Invalid TSS Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr11(){
    strcpy((int8_t*) msg, (const int8_t*) "Segment Not Present Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr12 (){
    strcpy((int8_t*) msg, (const int8_t*) "Stack Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr13 (){
    strcpy((int8_t*) msg, (const int8_t*) "General Protection Fault\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr14 (){
    uint32_t fault_addr;
    uint32_t errorNumber;
    asm volatile (
        "movl %%cr2, %%eax \n\
        movl %%eax, %0\n\
        popl %1 \n"\
        : "=r" (fault_addr), "=r"(errorNumber) \
        : \
        : "%eax"\
    );

    strcpy((int8_t*) msg, (const int8_t*) "Page Fault\nPage Address:0x");
    terminal_write(1, (const void*) msg, strlen(msg));
    itoa(fault_addr, (int8_t*) msg, 10);
    terminal_write(1, (const void*) msg, strlen(msg));
    strcpy((int8_t*) msg, (const int8_t*) "\nError Number:");
    terminal_write(1, (const void*) msg, strlen(msg));
    itoa(errorNumber, (int8_t*) msg, 10);
    terminal_write(1, (const void*) msg, strlen(msg));
    strcpy((int8_t*) msg, (const int8_t*) "\n");
    terminal_write(1, (const void*) msg, strlen(msg));

    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr15 (){
    strcpy((int8_t*) msg, (const int8_t*) "Reserved Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr16 (){
    strcpy((int8_t*) msg, (const int8_t*) "Floating-point error\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr17 (){
    strcpy((int8_t*) msg, (const int8_t*) "Alignment Check Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}

void isr18 (){
    strcpy((int8_t*) msg, (const int8_t*) "Machine Check Exception\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    isr_ret = EXCEP_RET;
    halt(0);
    return;
}


/*
* init_idt
*   DESCRIPTION: fills idt with exceptions, keyboard/rtc, & syscalls using idt vector & wrapper for handler
*   INPUTS:
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS:
*/
void init_idt(){
    printf("Initializing IDT\n");
    int idx;
    //exception interrupts filling IDT only for 0 through 18
    SET_IDT_ENTRY(idt[0], &isr0_wrapper);
    SET_IDT_ENTRY(idt[1], &isr1_wrapper);
    SET_IDT_ENTRY(idt[2], &isr2_wrapper);
    SET_IDT_ENTRY(idt[3], &isr3_wrapper);
    SET_IDT_ENTRY(idt[4], &isr4_wrapper);
    SET_IDT_ENTRY(idt[5], &isr5_wrapper);
    SET_IDT_ENTRY(idt[6], &isr6_wrapper);
    SET_IDT_ENTRY(idt[7], &isr7_wrapper);
    SET_IDT_ENTRY(idt[8], &isr8_wrapper);
    SET_IDT_ENTRY(idt[9], &isr9_wrapper);
    SET_IDT_ENTRY(idt[10], &isr10_wrapper);
    SET_IDT_ENTRY(idt[11], &isr11_wrapper);
    SET_IDT_ENTRY(idt[12], &isr12_wrapper);
    SET_IDT_ENTRY(idt[13], &isr13_wrapper);
    SET_IDT_ENTRY(idt[14], &isr14_wrapper);
    SET_IDT_ENTRY(idt[15], &isr15_wrapper);
    SET_IDT_ENTRY(idt[16], &isr16_wrapper);
    SET_IDT_ENTRY(idt[17], &isr17_wrapper);
    SET_IDT_ENTRY(idt[18], &isr18_wrapper);
    for(idx = 0; idx <= 18; idx++)
    set_table(idx);


    //2 cases for hardware interrups
    SET_IDT_ENTRY(idt[KB_ADDR], &KB_wrapper);         //keyboard interrupt
    set_table(KB_ADDR);
    SET_IDT_ENTRY(idt[RTC_ADDR], &RTC_wrapper);        //rtc interrupt
    set_table(RTC_ADDR);
    SET_IDT_ENTRY(idt[PIT_ADDR], &PIT_wrapper);        //pit interrupt
    set_table(PIT_ADDR);

    //0x80 case for sys call interrupt DPL = 3 & reserve3 = 1
    idt[SYSCALL_ADDR].reserved4 = 0;
    idt[SYSCALL_ADDR].reserved3 = 1;    //reserve3 = 1
    idt[SYSCALL_ADDR].reserved2 = 1;
    idt[SYSCALL_ADDR].reserved1 = 1;
    idt[SYSCALL_ADDR].reserved0 = 0;
    idt[SYSCALL_ADDR].dpl = 3;          //dpl set to 3
    idt[SYSCALL_ADDR].present = 1;
    idt[SYSCALL_ADDR].seg_selector = KERNEL_CS;
    idt[SYSCALL_ADDR].size = 1;
    SET_IDT_ENTRY(idt[SYSCALL_ADDR], &syscall_wrapper);
}

/*
* set_table
*   DESCRIPTION: according to documentation set struct element bit that corresponds
*   INPUTS:
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS:
*/
void set_table(int i){
    idt[i].reserved4 = 0;
    idt[i].reserved3 = 0;
    idt[i].reserved2 = 1;
    idt[i].reserved1 = 1;
    idt[i].reserved0 = 0;
    idt[i].dpl = 0;
    idt[i].present = 1;
    idt[i].seg_selector = KERNEL_CS;
    idt[i].size = 1;
}
