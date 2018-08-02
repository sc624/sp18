#include "rtc.h"

extern void test_interrupts();

// We have to use a flag to see if the current interrupt is skipped
volatile int8_t interrupt_happening = 0;

/*
 * RTC_init
 *   DESCRIPTION: This function enables the initializes the RTC chip
 *                by writing to its control registers and then
 *                sets the frequency determined by RTC_RATE (3-15)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows RTC send periodic interrupts
 */
void RTC_init(){
    cli(); // disabling interrupts

    // initializing the RTC

    outb(RTC_B, RTC_REG_PORT);         // select register B, and disable NMI
    uint8_t prev = inb(RTC_RW_PORT);   // read the current value of register B
    outb(RTC_B, RTC_REG_PORT);         // set the index again (a read will reset the index to register D)
    outb((prev|0x40), RTC_RW_PORT);    // write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(RTC_IRQ_LINE);

    // changing the frequency of the RTC
    outb(RTC_A, RTC_REG_PORT);                                                // set index to register A, disable NMI
    prev = inb(RTC_RW_PORT);                                                  // get initial value of register A
    outb(RTC_A, RTC_REG_PORT);
    int new_rate_of_frequency = INITIAL_RATE_OF_FREQUENCY;                    // reset index to A
    outb((prev & RTC_MASK) | new_rate_of_frequency, RTC_RW_PORT);             // setting frequency at rate 15

    sti();                                                                    // enabling interrupts
}

/*
 * RTC_handler
 *   DESCRIPTION: This function handles the interrupt service routine
 *                When there is a interrupt signal from the RTC. It calls
 *                test_interrupts and then reads from control register C
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints garbage to the screen
 */
extern void RTC_handler(){
    if(interrupt_happening == 0){
    	interrupt_happening = 1;
    }
    disable_irq(RTC_IRQ_LINE);  // disabling IRQ line to set up critical section
    //test_interrupts();          // printing out the garbage values

    send_eoi(RTC_IRQ_LINE);

    /*
    * Reading register C in order to avoid
    * receiving one interrupt from the RTC
    */
    outb(RTC_C, RTC_REG_PORT);	  // select register C
    inb(RTC_RW_PORT);					    // just throw away contents

    enable_irq(RTC_IRQ_LINE);     // end of critical section
}


/*
 * NMI_enable
 *   DESCRIPTION: This function enables non maskable interrutps
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables non maskable interrupts
 */
void NMI_enable(void){
	outb(RTC_REG_PORT, inb(RTC_REG_PORT)&0x7F);
}


/*
 * NMI_disable
 *   DESCRIPTION: This function disables non maskable interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: disables non maskable interrupts
 */

void NMI_disable(void){
	outb(RTC_REG_PORT, inb(RTC_REG_PORT)|0x80);
}

/*
 * RTC_open()
 * Description: This function initializes RTC frequency to 2HZ, return 0
 * INPUTS: fd
 * OUTPUTS: 0
 * RETURN VALUE: none
 * SIDE EFFECTS: Initializes rtc
 */
int32_t RTC_open(int32_t fd)
{
    pcb_t* pcb_ptr = get_pcb();
    pcb_ptr->fd_arr[fd].inode_num = 0;

    RTC_init();
    return 0;
}

/*
 * RTC_close()
 *Description: This function probably does nothing, unless RTC us virtualized , return 0
 *INPUTS: none
 *OUTPUTS: 0
 *RETURN VALUE: none
 *SIDE EFFECTS: -
 */
int32_t RTC_close(){
    return 0;
}

/*
* RTC_read()
*Description: This function probably does nothing, unless RTC us virtualized , return 0
*INPUTS: - fd: file descriptor
			 - buf: unused
			 - nbytes:bytes to write
*RETURN VALUE: 0
*SIDE EFFECTS: -
*/
int32_t RTC_read()
{
    interrupt_happening=0;
    // do nothing while there is no interrupt
    while(!interrupt_happening)
    {

    }
    interrupt_happening=0;

    return 0;
}

/*
* RTC_write()
*Description: This function must be able to change frequency, return 0 or -1.
*INPUTS: - fd: file descriptor
        - buf: unused
        - nbytes:bytes to write
*RETURN VALUE: 0
*SIDE EFFECTS: -
*/
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes)
{
     int8_t new_rate_of_frequency;//Based on the value of the buf, we will set the new frequency.
     // Look up the Table 3. Periodic Interrupt Rate and Square-Wave Output Frequency
     int32_t new_frequency;
     new_frequency = *(int32_t*)buf;
     if(new_frequency == 1024)
     {
    	 new_rate_of_frequency = 6;
     }
     else if (new_frequency ==512)
     {
    	 new_rate_of_frequency = 7;
     }
     else if (new_frequency == 256)
     {
    	 new_rate_of_frequency = 8;
     }
     else if (new_frequency == 128)
     {
    	 new_rate_of_frequency = 9;
     }

     else if (new_frequency == 64)
     {
    	 new_rate_of_frequency = 10;
     }
    else if (new_frequency == 32)
     {
    	 new_rate_of_frequency = 11;
     }
     else if (new_frequency == 16)
     {
    	 new_rate_of_frequency = 12;
     }
     else if (new_frequency == 8)
     {
    	 new_rate_of_frequency = 13;
     }
     else if (new_frequency == 4)
     {
    	 new_rate_of_frequency = 14;
     }
     else if (new_frequency == 2)
     {
    	 new_rate_of_frequency = 15;
     }
     else {
    return -1;
    }

    uint8_t prev = inb(RTC_RW_PORT);   // read the current value of register B
    outb(RTC_A, RTC_REG_PORT);                        // set index to register A, disable NMI
    prev = inb(RTC_RW_PORT);                          // get initial value of register A
    outb(RTC_A, RTC_REG_PORT);
    outb((prev & RTC_MASK) | new_rate_of_frequency, RTC_RW_PORT);  // setting frequency with the new rate
    return 0;
}
