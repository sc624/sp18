#include "syscalls.h"

/*
 * syscall 1 - 10
 *   DESCRIPTION: prints out syscall statement then halts
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: halt loop
 */

uint32_t pid_arr[MAX_RUNNING_PROCESSES];
static char msg[BUFFER_SIZE];
volatile int32_t isr_ret = 0;

typedef int32_t func();
/* directory file operations table */
int32_t dir_fops[4] = {      (int32_t)(dir_open),
                              (int32_t)(dir_close),
                              (int32_t)(dir_read),
                              (int32_t)(dir_write)};

/* file file operations table*/
int32_t file_fops[4] = {  (int32_t)(file_open),
                           (int32_t)(file_close),
                           (int32_t)(file_read),
                           (int32_t)(file_write)};
/*RTC operation table*/
int32_t rtc_fops[4] = {  (int32_t)(RTC_open),
                          (int32_t)(RTC_close),
                          (int32_t)(RTC_read),
                          (int32_t)(RTC_write)};
/*stdin & stdout operation table*/
int32_t terminal_fops[4] = { (int32_t)(terminal_open),
                           (int32_t)(terminal_close),
                           (int32_t)(terminal_read),
                           (int32_t)(terminal_write)};

/*
 * do_halt
 *   DESCRIPTION: Terminates the current process
 *   INPUTS: status - specified value to return to parent process
 *   OUTPUTS: none
 *   RETURN VALUE: Expand 8-bit argument from BL into 32-bit return value
 *                 to return to the parent program's execute syscall
 *   SIDE EFFECTS: If succesful, the current process is terminated and
 *                 parent data is restored
 */
int32_t do_halt(uint8_t status){
    cli();
    pcb_t* cur = get_pcb();
    pcb_t* parent = NULL;

    // Close any relevant FDs
    int fd;
    for(fd = 0; fd < MAX_OPEN_FILES; fd++)
        close(fd);

    // Free PID
    pid_arr[cur->pid] = NOT_USED;

    /* If current process is a child */
    if(cur->pid > THIRD_SHELL) {
        // Restore parent data
        parent = cur->parent;

        // Restore active pid
        terminals[cur->on_term].active_pid = parent->pid;

        // Restore parent paging
        tss.ss0 = KERNEL_DS;
        tss.esp0 = parent->kernel_stack;

        // Map 0x08000000 to 0x08400000 (128 to 132-MB) to (8MB + PARENT_PID * 4MB)
        page_directory[32] = (uint32_t) (FIRST_USER+(parent->pid)*FOUR_MB) | 0x87;
        flushTLB();
    } else {
        do_execute((const uint8_t*) "shell");
    }

    terminals[cur->on_term].vid_map_flag = 0;

    asm volatile("movl %0, %%ecx" : :"r"(cur->pid) :"ecx");
    /* Expand 8-bit argument to the parent program's execute call */
    asm volatile("movzb %0, %%eax" : :"r"(status) :"eax");

    asm volatile("jmp exec_return");

    return 0;
}

/*
 * do_execute
 *   DESCRIPTION: Load and execute a new program
 *   INPUTS: command - Space-separated sequence of words
 *                     The first word is the file name of the program
 *                     The rest of the words are arguments
 *   OUTPUTS: none
 *   RETURN VALUE: If       -1, the command cannot be executed
 *                    0 to 255, the program executes a halt syscall
 *                         256, the program dies by exception
 *   SIDE EFFECTS: If succesful, the new program is executed
 */
int32_t do_execute(const uint8_t* command){
    cli();
    /* Parse arguments*/
    const uint8_t* str = command;
    uint8_t cmd[BUFFER_SIZE];
    uint8_t args[BUFFER_SIZE];
    parse_args(str, cmd, args);

    /* Executable check */
    if(!command || !isExe(cmd))
        return -1;

    // Get free pid
    uint32_t pid = MAX_RUNNING_PROCESSES;
    int i;
    for(i = 0; i < MAX_RUNNING_PROCESSES; i++) {
        if(pid_arr[i] == NOT_USED) {
            pid = i;
            pid_arr[i] = USED;
            break;
        }
    }

    if(pid >= MAX_RUNNING_PROCESSES) {
        strcpy((int8_t*) msg, (const int8_t*) "Too many processes!\n");
        terminal_write(1, (const void*) msg, strlen(msg));
        return 0;
    }

    /* Set-up program paging */

    // Set up 1 4-MB page for task
    // Map 0x08000000 to 0x08400000 (128 to 132-MB) to (8MB + PID * 4MB)
    // Attributes: page size, user level, read/write, present
    page_directory[32] = (uint32_t) (FIRST_USER + pid * FOUR_MB)  | 0x87;

    // Flush TLB after page swap
    flushTLB();

    /* Load program */
    uint32_t entry = program_load((const uint8_t*) cmd);

    /* Create PCB (do not allocate) */
    uint32_t km_stack = (EIGHT_MB) - (pid * EIGHT_KB);
    pcb_t* task_pcb = (pcb_t*) (km_stack - EIGHT_KB);
    pcb_t tmp_pcb;
    init_pcb(pid,&tmp_pcb);

    memcpy((void*) task_pcb, (void*) &tmp_pcb, sizeof(pcb_t));
    strncpy((int8_t*) task_pcb->buffCopyArg, (const int8_t*) args, strlen((const int8_t*)args));

    /* Context switch */

    // Record which terminal the process is executing in

    // If this is a child process
    if(pid > THIRD_SHELL) {
        pcb_t* parent = (pcb_t*) get_pcb();
        task_pcb->parent = parent;
        terminals[curr_terminal].active_pid = pid;
        task_pcb->on_term = curr_terminal;
    }
    else {
        terminals[pid].active_pid = pid;
        task_pcb->on_term = pid;
    }

    // Modify TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = km_stack;
    task_pcb->kernel_stack = km_stack;

    int32_t ret = 0;
    isr_ret = 0;
    context_setup(entry,USER_STACK,pid);

    // Return 0 to 255 (from halt) on program halt
    asm volatile("movl %%eax, %0" :"=r" (ret));

    if(isr_ret == EXCEP_RET)
        return isr_ret;

    return ret;
}

/* Execute helper functions */

/*
 * init_pcb
 *   DESCRIPTION: Iniitializes PCB of the current process
 *   INPUTS: pid - Process ID (PID) number of process
 *           pcb - pointer to PCB of current process
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: If succesful, the PCB of the current process is modified
 */
int32_t init_pcb(uint32_t pid, pcb_t* pcb) {
    pcb->pid = pid;

    int i;
    /* Set all files to unused */
    for(i = 0; i < MAX_OPEN_FILES; i++) {
        pcb->fd_arr[i].flags = NOT_USED;
        pcb->fd_arr[i].fpos = 0;
    }

    /* Set stdin and stdout */
    pcb->fd_arr[0].flags = USED;
    pcb->fd_arr[0].fops = (int32_t*)terminal_fops;
    pcb->fd_arr[1].flags = USED;
    pcb->fd_arr[1].fops = (int32_t*)terminal_fops;

    return 0;
}

/*
 * parse_args
 *   DESCRIPTION: Parses the sequence of words passed into execute as command and arguments
 *   INPUTS: str - sequence of words passed into execute
 *           cmd - array to hold command word
 *          args - array to hold arguments
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: If succesful, the command and argument are written to cmd and
 *                 respectively.
 */
int32_t parse_args(const uint8_t* str, uint8_t* cmd, uint8_t* args) {
    memset(cmd, 0, BUFFER_SIZE);
    memset(args, 0, BUFFER_SIZE);

    /* Get and store command */
    int arg_start;
    for(arg_start = 0; (arg_start < strlen((const int8_t*)str)) && (str[arg_start] != ' '); arg_start++) {}
    strncpy((int8_t*) cmd, (int8_t*) str, arg_start);
    cmd[arg_start] = '\0';

    /* Get arguments and strip leading spaces*/
    str += arg_start;
    int count = 0;
    int i;
    for(i = 0; str[i]; i++) {
        if(str[i] != ' ' || '\0')
            args[count++] = str[i];
    }
    args[count] = '\0';

    return 0;
}

/*
 *   do_read
 *   DESCRIPTION: Read system call handler to dispatch to correct read file descriptor operation
 *                function
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fd is not valid or if buf or nbytes is null
 *                  returns amount of bytes read.
 *   SIDE EFFECTS: jumps to corresponding read function based on fd
 */
int32_t do_read (int32_t fd, void* buf, int32_t nbytes){
    sti();
    pcb_t* pcb_ptr = get_pcb();
    if (fd < 0 || fd > MAX_OPEN_FILES || buf == NULL || nbytes == NULL || pcb_ptr->fd_arr[fd].flags == NOT_USED)
    {
        return -1;
    }
    int32_t (*read_jump)(int32_t, const void*, int32_t) = (void*) pcb_ptr->fd_arr[fd].fops[READ];

    /* De-referencing function pointer to dispatch to fd's read operation */
    return read_jump(fd, buf, nbytes);
}

/*
 *   do_write
 *   DESCRIPTION: Write system call handler to dispatch to correct write file descriptor operation
 *                function.
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if fd is not valid or if buf or nbytes is null.
 *                  if inputs are valid it returns amount of bytes read.
 *   SIDE EFFECTS: jumps to corresponding write function based on fd
 */
int32_t do_write (int32_t fd, const void* buf, int32_t nbytes){
    sti();
    pcb_t* pcb_ptr = get_pcb();
    if (fd < 0 || fd > MAX_OPEN_FILES || buf == NULL || nbytes == NULL || pcb_ptr->fd_arr[fd].flags == NOT_USED)
    {
        return -1;
    }
    int32_t (*write_jump)(int32_t, const void*, int32_t) = (void*) pcb_ptr->fd_arr[fd].fops[WRITE];

    /* De-referencing function pointer to dispatch to fd's write operation */
    return write_jump(fd, buf, nbytes);
}

/*
 * do_open
 *   DESCRIPTION: provides access to the file system
 *   INPUTS: filename :     the name of the file we want to open.
 *   OUTPUTS: none
 *   RETURN VALUE: If       -1, the file cannot be open
 *                             fd value, when the file is open
 *   SIDE EFFECTS: If succesful, the file flag should be set to USED.
 */

int32_t do_open (const uint8_t* filename) {
    cli();
    int x;
    int fd=0;
    // First, extract the PCB
    pcb_t* pcb_ptr = get_pcb();
    // get the dentry
    dentry_t  open_dentry;
    /* Get the dentry information for this filename
    check if the filename has a directory entry
    */
    if(read_dentry_by_name(filename, &open_dentry) == -1)
        return -1;
    /*
    * Look for the next slot available in the file array and populate
    * file descriptor information for this f ile in the PCB.
    */

    for(x =2; x <8; x++)
    {
        //If the file is not in use
        if(pcb_ptr->fd_arr[x].flags == NOT_USED)
        {
            fd = x;
            break;
        }
    }

    if (fd == 0)
        return -1;

    // Let's check if it's a directory
    if(open_dentry.filetype == DIR_FILE)
    {
        pcb_ptr->fd_arr[fd].fops = (int32_t*)dir_fops;
        pcb_ptr->fd_arr[fd].flags = USED;
        ((func *)(dir_fops[OPEN]))(filename, fd);
    }
    // Let's check if it's a regular file
    else if(open_dentry.filetype == REG_FILE)
    {
        pcb_ptr->fd_arr[fd].fops = (int32_t*) file_fops;
        pcb_ptr->fd_arr[fd].flags = USED;
        ((func *)(file_fops[OPEN]))(filename, fd);
    }
    // Let's check if it's a rtc file
    else if (open_dentry.filetype == RTC_FILE)
    {
        pcb_ptr -> fd_arr[fd].fops = (int32_t*) rtc_fops;
        pcb_ptr->fd_arr[fd].flags = USED;
        ((func *)(rtc_fops[OPEN]))(fd);
    }

    return fd;
}
/*
 * do_close
 *   DESCRIPTION: closes the specified file descriptor and makes it available for
 *                return from later calls to open
 *   INPUTS: fd :  the fd value of the file to close.
 *   OUTPUTS: none
 *   RETURN VALUE: If       -1, for input 0 and 1 or invalid descriptor.
 *                           0 value, when the file is close
 *   SIDE EFFECTS: If succesful, the file flag should be set to NOT_USED.
 */
int32_t do_close (int32_t fd){
    cli();
    if( fd < 2 || fd> MAX_OPEN_FILES)
    {
        return -1;
    }
    // First, extract the PCB again
    pcb_t* pcb_ptr = get_pcb();
    /* If the file is already closed */
    if(pcb_ptr -> fd_arr[fd].flags == NOT_USED)
    {
        return -1;
    }
    /* Now we are facing an open file */
    pcb_ptr->fd_arr[fd].flags = NOT_USED;
    pcb_ptr->fd_arr[fd].fpos = 0;
    pcb_ptr->fd_arr[fd].inode_num = 0;
    int32_t (*close_jump)(int32_t) = (void*) pcb_ptr->fd_arr[fd].fops[CLOSE];

    return close_jump(fd);
}

/*
 * do_getargs
 *   DESCRIPTION: Reads the program's command line arguments into a user-level buffer
 *   INPUTS: buf - User-level buffer
 *        nbytes - Number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If succesful, write arguments to buffer
 */
int32_t do_getargs (uint8_t* buf, int32_t nbytes){
    cli();
    pcb_t* pcb_ptr = get_pcb();
    uint8_t* arguments = pcb_ptr->buffCopyArg;

    if(nbytes == 0 || arguments[0] == '\0' || buf == NULL)
    {
        return -1;
    }

    /*make sure the argument buffer is not smaller than what we are copying*/
    if( nbytes < strlen((const int8_t*)arguments))
    {
        return -1;
    }

    strcpy((int8_t*)buf, (const int8_t*)arguments);
    return 0;
}

/*
 * do_vidmap
 *   DESCRIPTION: Maps the text-mode video memory into user space at a pre-set virtual address
 *   INPUTS: screen_start - pointer to a variable that stores vidmem's virtual address
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: If succesful, write virtual address of user vidmem to screen_start
 */
int32_t do_vidmap (uint8_t** screen_start){
    cli();
    pcb_t* cur_task = get_pcb();

    /* Check if screen_start within user page */
    if(((uint32_t) screen_start < USER_PAGE_START) || ((uint32_t) screen_start > USER_PAGE_END))
        return -1;

    /* Map 4-kB page at 2-GB to video-memory */
    uint32_t vaddr = TWO_GB;

    /* Map 4-kB page (at 2-GB) to Video Memory
     * attributes: user level, read/write, present
     */
    terminals[cur_task->on_term].vid_map_flag = 1;
    user_vidmem_page_table[0] = (uint32_t) VIDEO_MEMORY | 0x7;
    flushTLB();

    *screen_start = (uint8_t*) vaddr;

    return 0;
}

/* Function doesn't do anything meaningful */
int32_t do_set_handler (int32_t signum, void* handler){
    strcpy((int8_t*) msg, (const int8_t*) "SET_HANDLER!\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    return -1;
}

/* Function doesn't do anything meaningful */
int32_t do_sigreturn (void){
    strcpy((int8_t*) msg, (const int8_t*) "SIGRETURN!\n");
    terminal_write(1, (const void*) msg, strlen(msg));
    return -1;
}
