#include <lcom/lcf.h>

#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>
#include "kbc.h"
#include "i8042.h"
#include "timer.h"
#include "i8254.h"
#include "lab2.h"
#include "timer.h"
#include "timer.c"

int sysCounter = 0; //Initializing counter for function sys_inb() calls
int kbcID = 0; //Initializing identification hook for keyboard configuration
uint8_t scancode;
int counter = 0;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int(kbd_test_scan)() {
  
  int ipc_status;
  message msg;
  uint32_t r;

  bool two_bytes = false;
  uint8_t bytes[2];

  uint8_t bit_no;
  if((kbc_subscribe_int(&bit_no)) != 0) // first subscribe the KBC interrupts
    return 1;

  // verify if a interrupt should be handled
  uint8_t irq_set = BIT(bit_no);

  while (scancode != ESC_BREAKCODE)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { //received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { //subscribed interrupt
            
            kbc_ih();

            if (two_bytes == true)
            {
            	two_bytes = false;
            	bytes[1] = scancode;
            	kbd_print_scancode(((scancode & MAKE_CODE) == 0),2,bytes);
            }
            
            else {
            	if (scancode == FIRST_BYTE_OF_2BYTES)
            	{
              		two_bytes = true;
              		bytes[0] = scancode;
              }
              else
              {
                bytes[0] = scancode;
              	kbd_print_scancode(((scancode & MAKE_CODE) == 0),1,bytes);
              }
          	}
          }
          break;
        default:
          break; //no other notifications expected: do nothing
      }
    }
  }
 
  if (kbc_unsubscribe_int() != 0)
    return 1;

  kbd_print_no_sysinb(sysCounter);
  
  return 0;
}

int(kbd_test_poll)() {

  bool two_bytes = false;
  uint8_t bytes[2];

	while (scancode != ESC_BREAKCODE) {
    tickdelay(micros_to_ticks(DELAY_US));
    kbc_ih();
        
    if(scancode == 0xff)
      continue;

    if (two_bytes == true)
    {
     	two_bytes = false;
     	bytes[1] = scancode;
     	kbd_print_scancode(((scancode & MAKE_CODE) == 0),2,bytes);
    }
    
    else
    {
     	if (scancode == FIRST_BYTE_OF_2BYTES)
     	{
       	two_bytes = true;
       	bytes[0] = scancode;
      }
            
      else
      {
       	bytes[0] = scancode;
       	kbd_print_scancode(((scancode & MAKE_CODE) == 0),1,bytes);
      }
    }
  }
  
  kbd_print_no_sysinb(sysCounter);

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  
  int ipc_status;
  message msg;
  uint32_t r;

  bool two_bytes = false;
  uint8_t bytes[2];

  uint8_t bit_no, bit_timer;
  if((kbc_subscribe_int(&bit_no)) != 0) return 1;
  if((timer_subscribe_int(&bit_timer)) != 0) return 1;

  // verify if a interrupt should be handled
  uint32_t kbc_irq_set = BIT(bit_no);
  uint32_t timer_irq_set = BIT(bit_timer);

  while (scancode != ESC_BREAKCODE && counter < n) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { //received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: // hardware interrupt notification
  
            if (msg.m_notify.interrupts & timer_irq_set) //subscribed interrupt
            { 
              timer_int_handler(); // handle interrupt

              if (interruptionCounter%sys_hz() == 0) { // 1sec passsed?
                counter++;
                printf("%d\n", counter);
                

            if (interruptionCounter%sys_hz() == 0) { // 1sec passsed?
              counter++;
              timer_print_elapsed_time();
            }
          }
            if (msg.m_notify.interrupts & kbc_irq_set) { //subscribed interrupt
              kbc_ih();
              counter = 0;
              interruptionCounter = 0;
              
              if (two_bytes == true)
              {

            	  two_bytes = false;
            	  bytes[1] = scancode;
            	  kbd_print_scancode(((scancode & MAKE_CODE) == 0),2,bytes);
                
              }
              
              else {

            	  if (scancode == FIRST_BYTE_OF_2BYTES)
                {
              		two_bytes = true;
              		bytes[0] = scancode;
                }
            
            	  else
                {
              		bytes[0] = scancode;
              		kbd_print_scancode(((scancode & MAKE_CODE) == 0),1,bytes);

                } // end of else
          	  } // end of the other else
            } // end of if kbc_irq_set
            break;
        default:
          break; //no other notifications expected: do nothing
         // end of if timed_irq_set
      } // end of switch
    } // end of is_ipc_notify
  } // end of while

  if (kbc_unsubscribe_int() != 0)
    return 1;
  if (timer_unsubscribe_int() != 0)
    return 1;

  kbd_print_no_sysinb(sysCounter);
  
  return 0;

}
