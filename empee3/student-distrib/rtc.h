#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "syscalls.h"

/* MAGIC NUMBERS brought to you by:
 * https://wiki.osdev.org/RTC
 */

#define RTC_REG_PORT	            0x70
#define RTC_RW_PORT		            0x71

#define RTC_A			                0x8A
#define RTC_B 			              0x8B
#define RTC_C			                0x8C

#define RTC_MASK		              0xF0
#define RTC_RATE		              0xF

#define RTC_IRQ_LINE	            8

#define INITIAL_RATE_OF_FREQUENCY 15

/*
 * RTC_init
 *   DESCRIPTION: This function enables the initializes the RTC chip
 *				  by writing to its control registers and then
 *				  sets the frequency determined by RTC_RATE (3-15)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows RTC send periodic interrupts
 */
void RTC_init();

/*
 * RTC_handler
 *   DESCRIPTION: This function handles the interrupt service routine
 				  When there is a interrupt signal from the RTC. It calls
 				  test_interrupts and then reads from control register C
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints garbage to the screen
 */
extern void RTC_handler();

/*
 * NMI_enable
 *   DESCRIPTION: This function enables non maskable interrutps
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables non maskable interrupts
 */
void NMI_enable(void);

/*
 * NMI_disable
 *   DESCRIPTION: This function disables non maskable interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: disables non maskable interrupts
 */
void NMI_disable(void);

/*
 * RTC_open()
 *Description: This function initializes RTC frequency to 2HZ, return 0
 *INPUTS: fd - file descriptor to open
 *OUTPUTS: 0
 *RETURN VALUE: none
 *SIDE EFFECTS: -
 */
 extern int32_t RTC_open(int32_t fd);

 /*
  * RTC_close()
  *Description: This function probably does nothing, unless RTC us virtualized , return 0
  *INPUTS: none
  *OUTPUTS: 0
  *RETURN VALUE: none
  *SIDE EFFECTS: -
  */
  extern int32_t RTC_close();
  /*
   * RTC_read()
   *Description: This function probably does nothing, unless RTC us virtualized , return 0
   *INPUTS: - fd: file descriptor
            - buf: unused
            - nbytes:bytes to write
   *RETURN VALUE: 0
   *SIDE EFFECTS: -
   */
   extern int32_t RTC_read();

   /*
   * RTC_write()
   *Description: This function must be able to change frequency, return 0 or -1.
   *INPUTS: - fd: file descriptor
            - buf: unused
            - nbytes:bytes to write
   *RETURN VALUE: 0
   *SIDE EFFECTS: -
   */
   extern int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
