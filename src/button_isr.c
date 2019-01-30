#include "button_isr.h"

// The TI includes complain if we do not include these two.
#include <stdbool.h>
#include <stdint.h>

#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include <stdio.h>

uint32_t move_again = 0;
uint32_t const WAIT = 0;
uint32_t const MOVE = 1;

void init_push_buttons(void)
{

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


	// interrupt setup, comment out the next 4 lines & polling & SWP interrupt work
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOIntRegister(GPIO_PORTF_BASE, sw1_isr);
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);

}

void sw1_isr(void)
{

	// Really poor debouncing at this point, but just wait in a loop briefly.
	int i = 0;
	for(i = 0; i < 1000000; i++);

	printf("Interrupt!!!!\n");
	fflush(stdout);
	move_again = MOVE;

	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);

}

void wait_on_push(void)
{
	while(move_again == WAIT);
	move_again = WAIT;
}

