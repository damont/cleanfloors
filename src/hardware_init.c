

#include "hardware_init.h"

// The TI includes complain if we do not include these two.
#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

void init_hardware(void)
{
	// Set up the system clock
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// Enable GPIO Port F for the push buttons
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}
