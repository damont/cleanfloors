//////////////////////////////////////////////////////////////////////////////
/// @file mapper.h
///
/// @author David Monts
///
/// @date April 2016
///
/// @brief Describes the abstract class for classes that implement the mapping.
//////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_MAPPER_H_
#define INCLUDE_MAPPER_H_

#include <path.h>

#include <deque>
#include <stdint.h>

struct Mapper_info
{
	int32_t m_x;
	int32_t m_y;
	std::deque<Path> m_map;

	Mapper_info() : m_x(0), m_y(0)
	{
		Path add_me(0, 0, STATE_CLEAR);
		m_map.push_back(add_me);
	}
};

//////////////////////////////////////////////////////////////////////////////
/// @class Mapper
///
/// @brief Provides the interface for the different mappers. THESE MAPPERS
///		   ARE NOT FAIL SAFE BUT DEPEND UPON THE ALGORITHMS TO KNOW HOW TO USE THEM.
///
/// @remarks Example of not fail safe. If a mapper that maps front and back
///			 is provided an x coordinate that does not yet exist in the map
///			 it will not bark. It will just ignore it.
//////////////////////////////////////////////////////////////////////////////
class Mapper
{

public:

	// Functions for creating the map
	virtual void move_and_update(Mapper_info& m_info, int32_t num_steps, State state) = 0;
	virtual void update_current(Mapper_info& m_info, State state) = 0;
	virtual void update_front(Mapper_info& m_info, State state) = 0;

	// Functions for traversing the map
	virtual int32_t next_object(Mapper_info& m_info) = 0;
	virtual void move(Mapper_info& m_info, int32_t num_steps) = 0;
};

#endif /* INCLUDE_MAPPER_H_ */
