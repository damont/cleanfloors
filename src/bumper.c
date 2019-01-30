/*
 * bumper.c
 *
 *  Created on: Jun 4, 2016
 *      Author: David
 */


#include "bumper.h"

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"


static int32_t Bumped = NO_CONTACT;

void enable_bumper()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);

	GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_2);
	GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_FALLING_EDGE);
	GPIOIntRegister(GPIO_PORTA_BASE, handle_bumper);

	GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_2);
}

void handle_bumper()
{

	Bumped = CONTACT;
	GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_2);

}

int32_t is_obstacle_present()
{
	return Bumped;
}

void clear_obstacle_reading()
{
	Bumped = NO_CONTACT;
	GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_2);
}

