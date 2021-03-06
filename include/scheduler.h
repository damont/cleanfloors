/*
 * scheduler.h
 *
 *  Created on: Jul 11, 2015
 *      Author: David
 *
 *  Scheduler class used to control the flow of the program
 */

#ifdef __cplusplus // included to ensure a c file does not translate this incorrectly

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_


#include "process.h"
#include <vector>


////////////////////////////////////////////////////////////////////////////////////
/// @class Scheduler
///
///	@brief Provides a round-robin scheduler. Implemented as a singleton class
////////////////////////////////////////////////////////////////////////////////////
class Scheduler
{

public:

	////////////////////////////////////////////////////////////////////////////////////
	/// @public get_instance
	///
	///	@brief returns THE instance of the scheduler
	///
	/// @returns Returns THE instance of the scheduler
	////////////////////////////////////////////////////////////////////////////////////
	static Scheduler& get_instance();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public run
	///
	///	@brief Goes through the execute processes for the processes in the process list.
	////////////////////////////////////////////////////////////////////////////////////
	void run();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public add_process
	///
	///	@brief Adds a process to the scheduler
	///
	/// @param add_me - Pointer to the process to add to the scheduler's process list
	////////////////////////////////////////////////////////////////////////////////////
	void add_process(Process * add_me);

	////////////////////////////////////////////////////////////////////////////////////
	/// @public remove_process
	///
	///	@brief Provides the ability to remove the process
	///
	/// @param process_name - Name of the process to remove from the process list. The
	///						  function does not actually remove the process but waits
	///						  until the process has finished before attempting removal
	////////////////////////////////////////////////////////////////////////////////////
	void remove_process(char const * process_name);
private:

	////////////////////////////////////////////////////////////////////////////////////
	/// @private Constructor
	///
	///	@brief Initializes the process list. Private because singleton implementation.
	////////////////////////////////////////////////////////////////////////////////////
	Scheduler();

	////////////////////////////////////////////////////////////////////////////////////
	/// @private actually_remove_process
	///
	///	@brief Checks to see if any processes were requested to be reomved. If so,
	///		   removes those processes from theprocess list
	////////////////////////////////////////////////////////////////////////////////////
	void actually_remove_process();

	std::vector<Process *> m_process_list;
};



#endif /* INCLUDE_SCHEDULER_H_ */

#endif // __cplusplus
