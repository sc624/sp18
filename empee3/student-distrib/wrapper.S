#define ASM 1

.globl isr0_wrapper
.globl isr1_wrapper
.globl isr2_wrapper, isr3_wrapper, isr4_wrapper, isr5_wrapper, isr6_wrapper, isr7_wrapper, isr8_wrapper, isr9_wrapper, isr10_wrapper, isr11_wrapper, isr12_wrapper, isr13_wrapper, isr14_wrapper, isr15_wrapper, isr16_wrapper, isr17_wrapper, isr18_wrapper
/*isr19_wrapper, isr20_wrapper, isr21_wrapper, isr22_wrapper, isr23_wrapper, isr24_wrapper, isr25_wrapper, isr26_wrapper, isr27_wrapper, isr28_wrapper, isr29_wrapper, isr30_wrapper, isr31_wrapper*/
.globl RTC_wrapper, KB_wrapper, PIT_wrapper
    isr0_wrapper:
        pusha            #push all registers
        call isr0        #may need to tweak this symbol depending on how the compiler mangles it
        popa             #restore all registers
        iret             #and return from exception

    isr1_wrapper:
        pusha
        call isr1
        popa
        iret

    isr2_wrapper:
        pusha
        call isr2
        popa
        iret

    isr3_wrapper:
        pusha
        call isr3
        popa
        iret

    isr4_wrapper:
        pusha
        call isr4
        popa
        iret

    isr5_wrapper:
        pusha
        call isr5
        popa
        iret

    isr6_wrapper:
        pusha
        call isr6
        popa
        iret

    isr7_wrapper:
        pusha
        call isr7
        popa
        iret

    //8 - Double fault (pushes an error code)
        isr8_wrapper:
        pusha
        call isr8
        popa
        add $4, %esp
        iret

    isr9_wrapper:
        pusha
        call isr9
        popa
        iret

    /*
    10 - Bad TSS (pushes an error code)
    11 - Segment not present (pushes an error code)
    12 - Stack fault (pushes an error code)
    13 - General protection fault (pushes an error code)
    14 - Page fault (pushes an error code)
    */
    isr10_wrapper:
        pusha
        call isr10
        popa
        add $4, %esp
        iret

    isr11_wrapper:
        pusha
        call isr11
        popa
        add $4, %esp
        iret

    isr12_wrapper:
        pusha
        call isr12
        popa
        add $4, %esp
        iret

    isr13_wrapper:
        pusha
        call isr13
        popa
        add $4, %esp
        iret

    isr14_wrapper:
        pusha
        call isr14
        popa
        add $4, %esp
        iret

    isr15_wrapper:
        pusha
        call isr15
        popa
        iret

    isr16_wrapper:
        pusha
        call isr16
        popa
        iret

    isr17_wrapper:
        pusha
        call isr17
        popa
        iret

    isr18_wrapper:
        pusha
        call isr18
        popa
        iret

    RTC_wrapper:
        pusha
        call  RTC_handler
        popa
        iret

    KB_wrapper:
        pusha
        call keyboard_handler
        popa
        iret

    PIT_wrapper:
        pusha
        call pit_handler
        popa
        iret
