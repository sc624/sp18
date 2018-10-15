/*
 * tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

#define MASK_2 0x2
#define MASK_4 0x4

#define DECIMAL 0x10//4th out of 7th MSB in representation
#define BYTEMASK 0x0F//bitmask 00001111
#define BITMASK_4 0xF // bitmask 1111

static int flag, ack, resend = 0;
static unsigned char arr[2];
// static spinlock_t lock = SPIN_LOCK_UNLOCKED;
unsigned long reset;

/************************ Protocol Implementation *************************/


int tuxctl_init (struct tty_struct* tty);
int tuxctl_buttons(unsigned long arg);
int tuxctl_set_led(struct tty_struct* tty, unsigned long arg);


/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.


    case for acks: bioc_event & reset & ack
 */

void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet) {
    unsigned a, b, c;
    // unsigned long flags;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];


    switch(a){
    	case MTCP_ACK:
        // spin_lock_irqsave(&lock, flags);            //spin lock & set acknoledge flag to 0
    		flag = 0;
        if (ack > 0) {
        ack--;                                      //decrease number of acks above 0
        }
        // printk("%d ", ack);
        // printk("%d ", resend);
        if(ack == 0 && resend == 1){              //require number of acks going out to going in (0) and when call reset, resend will equal 1
          tuxctl_set_led(tty, reset);             //reset leds
          resend = 0;                             //reset resend
        }
    		break;

    	case MTCP_BIOC_EVENT:
      flag = 1;
      // spin_lock_irqsave(&lock, flags);        //spinlock
  			arr[0] = b;                           //store controller packets into array vals
  			arr[1] = c;
      // spin_unlock_irqrestore(&lock, flags);
      flag = 0;
    		break;
    	case MTCP_RESET:
    		tuxctl_init(tty);                               //reinitialized & reset leds
          resend = 1;                                   //resend flag set to 1 to resend leds
    		return;
    	default:
    		return;
      }
}



int tuxctl_ioctl(struct tty_struct* tty, struct file* file,
                 unsigned cmd, unsigned long arg) {
/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
    switch (cmd) {
        case TUX_INIT:
          flag = 0;
          tuxctl_init(tty);
			    return 0;
        case TUX_BUTTONS:
          if(arg == 0)            //check if arg is null
            return -EINVAL;
          tuxctl_buttons(arg);
          return 0;
        case TUX_SET_LED:
          if(flag == 1)               //if spam flag true
            return -EINVAL;
          flag = 0;
          tuxctl_set_led(tty, arg);
          ack++;                      //increase num of acks
          return 0;
        default:
            return -EINVAL;
    }
}

/*
  tuxctl_init
  initializes tux controller by turning it on & initializing user mode
  input: port
*/

int tuxctl_init(struct tty_struct* tty){
  	char buf[2];
  	buf[0] = MTCP_BIOC_ON;               //initialize Enable Button interrupt-on-change
  	buf[1] = MTCP_LED_USR;               //Put the LED display into user-mode
    ack = 2;                              //number of acks = 2 (MTCP_BIOC_ON & MTCP_LED_USR)
  	tuxctl_ldisc_put(tty, buf, 2);       //send to controller
  return 0;
}


/*
  tuxctl_set_led
  turns on leds segments on controller display
  intput: arg--# to be displayed & which leds and decimals
          tty--ports
  output: 0 = success; -einval = failure
  led = 16:23
  decimal = 24:31
*/
int tuxctl_set_led(struct tty_struct* tty, unsigned long arg){
  char disp[16] = {0xE7, 0x06,  0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};  //display array for led segments
  int idx;
  unsigned char buffy[6];           //6 byte buffer delivered to controller
  unsigned long arg2 = arg;         //temp var
  buffy[0] = MTCP_LED_SET;          //set 1st val of buffer to set led
  buffy[1] = 0xFF;                  //second val of buffer
  // spin_lock_irqsave(&lock, flags);  //need to spin lock
  for(idx = 0; idx < 4 ; idx++){    //4 for total number of bytes to be displayed
    arg2 = arg >> (idx * 4);        //shift for each number to be displayed
    arg2 = arg2 & BITMASK_4;        //mask out everything except number to be displayed
		buffy[idx+2] = disp[arg2];      //set corresponding led to be displayed
    if(((arg >> (idx + 24)) & 1) == 1 && ((arg >> (idx + 16)) & 1) == 1)        //check if led low bits & decimal low byes are active
      buffy[idx+2] = buffy[idx+2] + 0x10;
    else if(((arg >> (idx + 16)) & 1) != 1)                                     //check if led byte is on
      buffy[idx+2] = 0;
  }

	tuxctl_ldisc_put(tty, buffy, 6);     //put call size 6
  reset = arg;                         //set reset val as previous arg
  // spin_unlock_irqrestore(&lock, flags);   //spin unlock restore flags
	return 0;
}


/*
  tuxctl_buttons -- processes button interrupt inputs & sets corresponding bits of button's low bytes
  input: arg -- pointer to 32bit int
  output: successful or not
*/
int tuxctl_buttons(unsigned long arg){
  unsigned char eight_bits;       //butt = button
  unsigned char butts1 = BITMASK_4 & arr[0];       //bits sent to tux first set as C, B, A, Start
  unsigned char butts2 = BITMASK_4 & arr[1];        //bits sent to tux include right down left up

  unsigned char buttons = butts1 << 1;              //bit shift right/left bit masks to check respective bits in data bits
  unsigned char direction = butts2 >> 1;
  if (arg == 0)                   //check is arg is null
    return -EINVAL;

  eight_bits = butts2 | buttons | direction;      //add data bits to send to driver
  eight_bits = butts2 << 4;                       //up left down right offset
  butts1 |= eight_bits;                           //cbas + direction bits
  copy_to_user((void*)arg, (void*)(&butts1), sizeof(unsigned char));    //check is copy_to_user was successful
  return 0;
}
