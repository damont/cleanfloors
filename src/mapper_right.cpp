//////////////////////////////////////////////////////////////////////////////
/// @file mapper_right.cpp
///
/// @author David Monts
///
/// @date April 2016
///
/// @see mapper_right.h
//////////////////////////////////////////////////////////////////////////////

#include "mapper_right.h"
#include <stdio.h>

void Mapper_right::move_and_update(Mapper_info& m_info, int32_t num_steps, State state)
{

	int32_t added_steps = 0;

	// Search in the deque and get the Path we need.
	for (std::deque<Path>::iterator itty = m_info.m_map.begin();
			(itty != m_info.m_map.end()) && (added_steps < num_steps);
			++itty)
	{	// Once we move past the x position, we want to set the state
		if(((*itty).get_id() > m_info.m_x) &&
		   (added_steps < num_steps))
		{
			(*itty).set_state(m_info.m_y, state);
			added_steps++;
			printf("Added step right");
			fflush(stdout);
		}
	}

	// We need to add paths where we do not have them yet.
	Path next_path(m_info.m_x + added_steps + 1, m_info.m_y, state);
	while (added_steps < num_steps)
	{
		m_info.m_map.push_back(next_path);
		added_steps++;
		printf("Added step right");
		fflush(stdout);
		next_path.set_id(next_path.get_id() + 1);
	}

	// Added steps should always equal num_steps
	m_info.m_x = m_info.m_x + added_steps;
}

void Mapper_right::update_current(Mapper_info& m_info, State state)
{

}

void Mapper_right::update_front(Mapper_info& m_info, State state)
{
	int32_t x_update = m_info.m_x + 1;
	bool is_updated = false;

	// Search in the deque and get the Path we need.
	for (std::deque<Path>::iterator itty = m_info.m_map.begin();
			itty != m_info.m_map.end();
			++itty)
	{	// Once we move past the x position, we want to set the state
		if((*itty).get_id() == x_update)
		{
			(*itty).set_state(m_info.m_y, state);
			is_updated = true;
		}
	}

	// If we do not find our path already existing in the map, then we need to
	// add a path to the front.
	if(!is_updated)
	{
		Path path_to_add(x_update, m_info.m_y, state);
		m_info.m_map.push_front(path_to_add);
	}

}

int32_t Mapper_right::next_object(Mapper_info& m_info)
{
	int32_t clear_steps = 0;

	// Search in the deque and get the Path we need.
	for (std::deque<Path>::iterator itty = m_info.m_map.begin();
			itty != m_info.m_map.end();
			++itty)
	{	// Once we move past the x position, we want to set the state
		if(((*itty).get_id() > m_info.m_x) &&
		   ((*itty).get_state(m_info.m_y) == STATE_CLEAR))
		{
			clear_steps++;
		}
	}

	return clear_steps;
}

void Mapper_right::move(Mapper_info& m_info, int32_t num_steps)
{
	m_info.m_x = m_info.m_x + num_steps;
}
