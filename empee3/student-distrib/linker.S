.globl syscall_wrapper

#define KERNEL_DS   0x18

  /*.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
             1          2       3     4      5     6      7         8         9           10 */
syscall_wrapper:
    pushfl                  #load flags & registers
    pushal

    cmp $1,   %eax          #syscall num -- check if less than 1
    jl invalid

    cmp $0xA, %eax          #check if greater than 10
    jg invalid

    jmp     continue

invalid:
    movl $-1, temp        #return -1
    jmp end

continue:
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    movw $KERNEL_DS, %si        #kernel mode
    movw %si, %ds

    cmpb $0x01, %al
    je halt_call

    cmpb $0x02, %al
    je execute_call

    cmpb $0x03, %al
    je read_call

    cmpb $0x04, %al
    je write_call

    cmpb $0x05, %al
    je open_call

    cmpb $0x06, %al
    je close_call

    cmpb $0x07, %al
    je getargs_call

    cmpb $0x08, %al
    je vidmap_call

    cmpb $0x09, %al
    je set_handler_call

    cmpb $0x0A, %al
    je sigreturn_call

  halt_call:
    call do_halt
    jmp retval

  execute_call:
    call do_execute
    jmp retval

  read_call:
    call do_read
    jmp retval

  write_call:
    call do_write
    jmp retval

  open_call:
    call do_open
    jmp retval

  close_call:
    call do_close
    jmp retval

  getargs_call:
    call do_getargs
    jmp retval

  vidmap_call:
    call do_vidmap
    jmp retval

  sigreturn_call:
    call do_sigreturn
    jmp retval

  set_handler_call:
    call do_set_handler
    jmp retval




    /* eax has ret val and return from func call */
retval:
    movl    %eax, temp          #store ret val
    addl    $12, %esp           #restore stack ptr
end:
    popal                       #restore flags & registers
    popfl
    movl    temp, %eax          #load eax w/ ret val
    iret

	.word   KERNEL_DS
temp:
    .long   1
