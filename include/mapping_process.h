/*
 * mapping_process.h
 *
 *  Created on: Aug 4, 2015
 *      Author: David
 */

#ifndef INCLUDE_MAPPING_PROCESS_H_
#define INCLUDE_MAPPING_PROCESS_H_

#include "process.h"
#include "map.h"

class Mapping_process : public Process
{
public:
	virtual void initialize();
	virtual void execute();
private:
	Map m_map;

	void navigate(Directions_list);
	void create_map();

	bool move_forward();
	bool move_across();
	void turn_right();
	void turn_left();
};



#endif /* INCLUDE_MAPPING_PROCESS_H_ */
