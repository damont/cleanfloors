/*
 * process.h
 *
 *  Created on: Jul 11, 2015
 *      Author: David
 *
 *      Base class for different processes. This class will only ever serve
 *      as a base class. Purely virtual.
 */

#ifndef INCLUDE_PROCESS_H_
#define INCLUDE_PROCESS_H_

class Process
{
public:
	virtual void initialize() = 0;
	virtual void execute() = 0;
private:
};


#endif /* INCLUDE_PROCESS_H_ */
