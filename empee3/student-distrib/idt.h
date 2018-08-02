#ifndef IDT_H
#define IDT_H


#include "x86_desc.h"
#include "types.h"
#include "lib.h"
#include "terminal.h"

#define SYSCALL_ADDR        0x80
#define RTC_ADDR            0x28
#define KB_ADDR             0x21
#define EXCEP_RET           256
#define PIT_ADDR            0x20

extern void init_descriptor_tables();

extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();

#endif
