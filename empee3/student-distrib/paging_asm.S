.globl loadPageDirectory, enablePaging, flushTLB

loadPageDirectory:
    push %ebp
    mov %esp, %ebp

    /* Loads first entry of page directory to CR3 (PDBR) */
    mov 8(%esp), %eax
    mov %eax, %cr3

    mov %ebp, %esp
    pop %ebp
    ret

enablePaging:
    push %ebp
    mov %esp, %ebp

    /* Set bit 4 (Page Size Extension) of CR4 to 1
     * When the PSE flag in CR4 is set, both 4-MByte pages
     * and page tables for 4-KByte pages can  be accessed
     * from the same page directory.
     */
    mov %cr4, %eax
    or $0x10, %eax
    mov %eax, %cr4

    /* Set bit 31 (paging) of CR0 to 1 */
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0


    mov %ebp, %esp
    pop %ebp
    ret

flushTLB:
    push %ebp
    mov %esp, %ebp

    /* Write to the page directory base register (CR3) to flush TLB
     * https://wiki.osdev.org/TLB
     */
    mov	%cr3,%eax
	mov	%eax,%cr3

    mov %ebp, %esp
    pop %ebp
    ret
