//////////////////////////////////////////////////////////////////////////////
/// @file mapper_front.h
///
/// @author David Monts
///
/// @date April 2016
///
/// @brief Implements the mapping functionality when the heading is frontwards.
//////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_MAPPER_FRONT_H_
#define INCLUDE_MAPPER_FRONT_H_

#include "mapper.h"

class Mapper_front : public Mapper
{
public:
	// Functions for creating the map
	virtual void move_and_update(Mapper_info& m_info, int32_t num_steps, State state);
	virtual void update_current(Mapper_info& m_info, State state);
	virtual void update_front(Mapper_info& m_info, State state);

	// Functions for traversing the map
	virtual int32_t next_object(Mapper_info& m_info);
	virtual void move(Mapper_info& m_info, int32_t num_steps);
};



#endif /* INCLUDE_MAPPER_FRONT_H_ */
