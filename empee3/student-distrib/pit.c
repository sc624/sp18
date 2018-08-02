#include "pit.h"

/*
The PIT implememntation is based up on this documentation:
https://wiki.osdev.org/Programmable_Interval_Timer

Note that we only referred to it in order to get correct values and port.
*/

// Global counter for terminal
int terminal_cycle;

/*
 * pit_init()
 *   DESCRIPTION: Initialize PIT frequency to 29,829 Hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables PIT interrupts
 */
void pit_init()
{
    cli();
    outb(MAX_FREQ_PIT/COUNTER_DIVIDER & MASK_FREQ, CHANNEL_0_RW_PIT);
    outb(MAX_FREQ_PIT/COUNTER_DIVIDER >> SHIFT_BIT, CHANNEL_0_RW_PIT);
    outb(MODE_2, COMMAND_RGSTR_PIT);
    enable_irq(IRQ_PIT);
    sti();
}

/*
 * pit_handler
 *   DESCRIPTION: Handle PIT interrupts (for round-robin scheduling)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Schedule processes
 */
void pit_handler()
{
  send_eoi(IRQ_PIT);
  schedule();
}
