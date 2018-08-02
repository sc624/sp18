/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
static unsigned int cached_irq_mask = 0xffff;

#define cached_master_mask (cached_irq_mask)
#define cached_slave_mask (cached_irq_mask >>8)

/*
  * i8259_init()
  *
  * Description:
  * Initialization of the 8259 PIC. Corrects bytes are being sent to the controller as
  * as written in the datasheet.
  *
  * Inputs: none
  *
  * Retvals: none
  */
void i8259_init(void) {
    /* Mask all the IRQ lines */
    master_mask = 0xff;
    slave_mask = 0xff;
    outb(master_mask, MASTER_8259_DATA_PORT);       /* mask all of 8259A-1*/
    outb(slave_mask,  SLAVE_8259_DATA_PORT);  /*  mask all of 8259A-2*/

    outb(ICW1, MASTER_8259_PORT); /* ICW1: select 8259A-1 init*/
    outb(ICW2_MASTER , MASTER_8259_DATA_PORT);/* ICW2: 8259A-1 IRO-7 mapped to 0x20-0x27*/

    outb(ICW3_MASTER, MASTER_8259_DATA_PORT);/*8259A-1 (the master) has a slave on IR2*/

    outb(ICW4, MASTER_8259_DATA_PORT);/*master expects normal EOI*/

    outb(ICW1, SLAVE_8259_PORT);/*ICW1: select 8259A-2 init*/
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);/* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f*/
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);/*8259A-2 is a slave on master's IR2*/
    outb(ICW4, SLAVE_8259_DATA_PORT);/*(slave's support for AEOI in flat mode is to be investogate)*/

    outb(cached_master_mask, MASTER_8259_DATA_PORT); /*restore master IRQ mask*/
    outb(cached_slave_mask, SLAVE_8259_DATA_PORT); /* restore slave IRq mask*/

    enable_irq(2);
}
/*
   * enable_irq()
   *
   * Description:
   * Enables (e.g. unmasks) the specified IRQ with an ACTIVE LOW bitmask. Writes
   *an 8-bit value the mask (master or slave) which corresponds to
   * the integer input: irq_num.
   *
   * Inputs: irq_num - IRQ line
   *
   * Retvals: none
   */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    if (irq_num <8 && irq_num >= 0)
    {
        master_mask &= ~(1<< irq_num);
        value = master_mask;
        port = MASTER_8259_DATA_PORT;
    }
    else{
        slave_mask &= ~(1<< (irq_num-8));
        port = SLAVE_8259_DATA_PORT;
        //irq_num -= 8;
        value = slave_mask;
    }

    outb(value,port);
}

   /*
  * disable_irq()
  *
  * Description:
  * Disable (e.g. masks) the specified IRQ with an INACTIVE HIGH bitmask. Writes
  * an 8-bit value the mask (master or slave) which corresponds to
  * the integer input: irq_num.
  *
  * Inputs: irq_num - IRQ line
  *
  * Retvals: none
  */
void disable_irq(uint32_t irq_num) {

    uint16_t port;
    uint8_t value;
    if(irq_num <8 && irq_num >= 0)
    {

        port = MASTER_8259_DATA_PORT;
        master_mask |= (1<< irq_num);
        value = master_mask;
    }
    else{
        port = SLAVE_8259_DATA_PORT;
        slave_mask |=(1<<(irq_num-8));
        value = slave_mask;
    }

    outb(value,port);

}


 /*
 * send_eoi()
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *   INPUTS: irq_num - IRQ line
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
void send_eoi(uint32_t irq_num) {
    /*This is issued to the PIC chips at the end of an IRQ-based interrupt routine.*/

    uint16_t port;
    uint8_t value;
    uint16_t port2;
    uint8_t value2;
    uint16_t port3;
    uint8_t value3;
    /*This is issued to the PIC chips at the end of an IRQ-based interrupt routine.*/

    /*If the IRQ came from the Master PIC, it is sufficient to issue this command only to the Master PIOC*/
    /*If the IRQ came from the Slave PIC, it is necessary to issue the command to both PIC chips*/
    if(irq_num <16 && irq_num >= 8)
    {
        value = EOI| (irq_num-8);
        value2 = EOI|2;
        port = SLAVE_8259_PORT;
        port2 = MASTER_8259_PORT;
        outb(value, port);
        outb(value2, port2);
    }
    else
    {
        value3 = EOI|irq_num;
        port3 = MASTER_8259_PORT;
        outb(value3, port3);
    }
}
