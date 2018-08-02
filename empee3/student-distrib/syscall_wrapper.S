#define ASM 1

.globl halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

/*
SYSCALL wrapper
store ebx and load eax w/ correct syscall number (halt = 1, execute = 2, etc)
load ebx, ecx, & edx with params for syscall_wrapper
restore ebx & return
*/

  halt:
    pushl %ebx
    movl $1, %eax           #syscall number
    movl 8(%esp), %ebx
    int $0x80
    popl	%ebx
    ret

  execute:
    pushl %ebx
    movl $2, %eax           #syscall number
    movl 8(%esp), %ebx
    int $0x80
    popl	%ebx
    ret

  read:
    pushl %ebx
    movl $3, %eax           #syscall number
    movl 8(%esp), %ebx
    movl	12(%esp),%ecx
    movl	16(%esp),%edx
    int $0x80
    popl	%ebx
    ret

  write:
    pushl %ebx
    movl $4, %eax           #syscall number
    movl 8(%esp), %ebx
    movl	12(%esp),%ecx
    movl	16(%esp),%edx
    int $0x80
    popl	%ebx
    ret

  open:
    pushl %ebx
    movl $5, %eax           #syscall number
    movl 8(%esp), %ebx
    int $0x80
    popl	%ebx
    ret

  close:
    pushl %ebx
    movl $6, %eax           #syscall number
    movl 8(%esp), %ebx
    int $0x80
    popl	%ebx
    ret

  getargs:
    pushl %ebx
    movl $7, %eax           #syscall number
    movl 8(%esp), %ebx
    movl	12(%esp),%ecx
    int $0x80
    popl	%ebx
    ret

  vidmap:
    pushl %ebx
    movl $8, %eax           #syscall number
    movl 8(%esp), %ebx
    int $0x80
    popl	%ebx
    ret

  set_handler:
    pushl %ebx
    movl $9, %eax           #syscall number
    movl 8(%esp), %ebx
    movl	12(%esp),%ecx
    int $0x80
    popl	%ebx
    ret

  sigreturn:
    pushl %ebx
    movl $10, %eax           #syscall number
    movl 8(%esp), %ebx
    movl	12(%esp),%ecx
    movl	16(%esp),%edx
    int $0x80
    popl	%ebx
    ret
