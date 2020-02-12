#ifndef _LCOM_KBC_H_
#define _LCOM_KBC_H_

#include <lcom/lcf.h>
#include <stdint.h>
#include "i8042.h"

void delay();

int kbc_subscribe_int(uint8_t *bit_no);
int kbc_unsubscribe_int();

int sys_inb_count(port_t port, uint32_t *byte);
int util_sys_inb_kbc(port_t port, uint8_t *byte);

void (kbc_ih)();

#endif /* _LCOM_KBC_H */
