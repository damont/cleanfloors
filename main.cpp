/*
 * main.cpp
 */

#include "scheduler.h"
#include "mapping_process.h"
#include "navigation.h"
#include "hardware_init.h"
#include "button_isr.h"
#include "bumper.h"

void main(void) {

	init_hardware();
	init_push_buttons();

	//int loop = 0;
	//for (loop = 0; loop <7000000; loop++){}

	// Initialize the hardware to run the wheels and distance sensors
	Navigation_interface::init_hardware();
	enable_bumper();

	Mapping_process * mp = new Mapping_process();
	Scheduler &inst = Scheduler::get_instance();

	inst.add_process((Process *)mp);

	while(1)
	{

		inst.run();

	}

}
