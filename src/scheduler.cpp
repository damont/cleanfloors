/*
 * scheduler.cpp
 *
 *  Created on: Jul 11, 2015
 *      Author: David
 */

#include "scheduler.h"
#include <stdint.h>

Scheduler::Scheduler()
{

}


////////////////////////////////////////////////////////////////////////////////////
/// @public get_instance
///
///	@brief returns THE instance of the scheduler
///
/// @returns Returns THE instance of the scheduler
////////////////////////////////////////////////////////////////////////////////////
Scheduler& Scheduler::get_instance()
{
	static Scheduler instance;

	return instance;
}

////////////////////////////////////////////////////////////////////////////////////
/// @public run
///
///	@brief Goes through the execute processes for the processes in the process list.
////////////////////////////////////////////////////////////////////////////////////
void Scheduler::run()
{

	// For each process I want to go through run the initialization function and then
	// go through and loop through the execute functions forever
	for (uint32_t process_index = 0; process_index < m_process_list.size(); process_index++)
	{
		m_process_list[process_index]->initialize();
	}

	while(1)
	{
		for (uint32_t process_index = 0; process_index < m_process_list.size(); process_index++)
		{
			m_process_list[process_index]->execute();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
/// @public add_process
///
///	@brief Adds a process to the scheduler
///
/// @param add_me - Pointer to the process to add to the scheduler's process list
////////////////////////////////////////////////////////////////////////////////////
void Scheduler::add_process(Process * add_me)
{
	if (add_me != NULL)
	{
		m_process_list.push_back(add_me);
	}
}

void Scheduler::remove_process(char const * process_name)
{

}


