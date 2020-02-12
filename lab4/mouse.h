#ifndef _LCOM_MOUSE_H_
#define _LCOM_MOUSE_H_

#include <lcom/lcf.h>
#include <stdint.h>
#include "i8042.h"

//extern struct mouse_ev m_ev; //Struct that contains a mouse event and mouse coordinates x and y

typedef enum state_t //Enumeration with all states of state machine
{
  INITIAL_STATE,        //Initial state
  LEFT_LINE,       //Left line of inverted V
  VERTEX,      //Vertex of inverted V
  RIGHT_LINE,       //Right line of inverted V
  FINAL_STATE        //Final state
}state_t;

typedef struct state_machine_t //Struct that works as a state machine
{
  state_t currentState; //Current state of the state machine
  int x_displacement; //Current displacement in the x direction
  int y_displacement; //Current displacement in the y direction
}state_machine_t;

int mouse_subscribe_int(uint8_t *bit_no);
int mouse_unsubscribe_int();

void (mouse_ih)();

void packet_create();

int write_mouse(uint32_t command);
int read_kbc(uint32_t * data);
int write_kbc(uint32_t port, uint32_t cmd);
int write_cmdbyte(uint32_t cmd);

void detect_mouse_event(struct packet *pp, state_machine_t *st_machine, struct mouse_ev *m_ev);
void handle_mouse_event(state_machine_t *st_machine, struct mouse_ev event, uint8_t x_len, uint8_t tolerance);
int check_gesture(uint8_t x_len, uint8_t tolerance, state_machine_t *st_machine, struct mouse_ev event);

#endif /* _LCOM_MOUSE_H */
