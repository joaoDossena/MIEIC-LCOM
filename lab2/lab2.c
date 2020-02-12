#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

extern int timerID;
extern unsigned int interruptionCounter;


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
  uint8_t st;
  
  int error1 = timer_get_conf(timer, &st); //Tests for first error
  if (error1 != 0)
    return error1; //If unsuccessful
  
  int error2 = timer_display_conf(timer, st, field); //Tests for second error
  if (error2 != 0)
    return error2; //If unsuccessful

  return 0; //If successful
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {

  int error = timer_set_frequency(timer, freq); //Tests for error
  
  return error; //Returns 1 upon failure, 0 upon success
}

int(timer_test_int)(uint8_t time) {

  int ipc_status;
  message msg;
  uint32_t r;

  uint8_t bit_no;
  if((timer_subscribe_int(&bit_no)) != 0)
    return 1;

  // verify if a interrupt should be handled
  uint32_t irq_set = BIT(bit_no);

  while( time /*?*/ ) {
    /* Get a request message */
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { //received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { //subscribed interrupt
            timer_int_handler(); // handle interrupt

            if (interruptionCounter == 0) { // 1sec passsed?
              time--;
              timer_print_elapsed_time();
            }
          }
          break;
        default:
          break; //no other notifications expected: do nothing
      }
    } else { //received a standard message, not a notification
      // no standard messages expected: do nothing
    }
  }

  if (timer_unsubscribe_int() != 0)
    return 1;

  return 0;
}