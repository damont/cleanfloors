
#ifndef BUTTON_ISR_H
#define BUTTON_ISR_H

#include <stdint.h>

extern uint32_t move_again;
extern uint32_t const WAIT;
extern uint32_t const MOVE;

#ifdef __cplusplus
extern "C" {
#endif

void init_push_buttons(void);
void sw1_isr(void);
void wait_on_push(void);

#ifdef __cplusplus
}
#endif


#endif
