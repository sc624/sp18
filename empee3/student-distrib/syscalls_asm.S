.globl context_setup,get_pcb,exec_return

/* Does not follow standard C-calling convention */
context_setup:
    /* Get PID */
    mov     12(%esp),%ecx

    /* Find where to save kernel stack */
    lea     kernel_stack,%eax
    lea     kernel_base,%ebx

    lea     (%eax,%ecx,4),%eax
    lea     (%ebx,%ecx,4),%ebx

    /* Save kernel stack */
    mov     %esp, (%eax)
    mov     %ebp, (%ebx)

    /* Get user stack */
    mov     8(%esp),%ebx

    /* Set ds to USER_DS
     * SS and CS is handled by IRET
     */
    mov     $0x2B,%eax
    mov     %ax,%ds

    /* Save EIP */
    mov     4(%esp),%eax

    /* Push SS,ESP,EFLAGS,CS,EIP */
    push    $0x2B
    push    %ebx
    pushf
    push    $0x23
    push    %eax

    iret
exec_return:
    /* Save return value */
    mov    %eax,return_val

    /* Find where to load kernel stack
     * Note: PID is in ECX
     */
    lea     kernel_stack,%eax
    lea     kernel_base,%ebx

    lea     (%eax,%ecx,4),%eax
    lea     (%ebx,%ecx,4),%ebx

    /* Restore kernel stack */
    mov     (%eax),%esp
    mov     (%ebx),%ebp

    /* Restore return value */
    mov     return_val,%eax

    ret

kernel_stack:
    .long   1
    .long   1
    .long   1
    .long   1
    .long   1
    .long   1
kernel_base:
    .long   1
    .long   1
    .long   1
    .long   1
    .long   1
    .long   1
return_val:
    .long   1

get_pcb:
    push    %ebp
    mov     %esp,%ebp

    /* Get kernel mode stack and AND with mask */
    mov     %esp,%eax
    and     $0xFFFFE000,%eax

    mov     %ebp,%esp
    pop     %ebp

    ret
