//////////////////////////////////////////////////////////////////////////////
/// @file mapper_front.cpp
///
/// @author David Monts
///
/// @date April 2016
///
/// @see mapper_front.h
//////////////////////////////////////////////////////////////////////////////

#include "mapper_front.h"
#include <stdio.h>

void Mapper_front::move_and_update(Mapper_info& m_info, int32_t num_steps, State state)
{
	// Search in the deque and get the Path we need.
	for (std::deque<Path>::iterator itty = m_info.m_map.begin();
			itty != m_info.m_map.end();
			++itty)
	{
		if((*itty).get_id() == m_info.m_x)
		{ 	// If the Path is found, we need to set the states for everything
			// in between us and the num steps.
			for(int32_t step = 1; step <= num_steps; step++)
			{
				(*itty).set_state(m_info.m_y + step, state);
				printf("Added step front");
				fflush(stdout);
			}

			// Lastly we need to advance the y position.
			m_info.m_y = m_info.m_y + num_steps;
		}
	}
}

void Mapper_front::update_current(Mapper_info& m_info, State state)
{

}

void Mapper_front::update_front(Mapper_info& m_info, State state)
{
	// So I need to look at the deque, and see what is in the current location. What
	// are my options? Well, it's possible that there is nothing the current location
	// and then it is also
	for (std::deque<Path>::iterator itty = m_info.m_map.begin();
			itty != m_info.m_map.end();
			++itty)
	{
		if((*itty).get_id() == m_info.m_x)
		{ 	// If the Path is found, then we just need to set the state at the current
			// y location plus 1 since we are updating the front!!
			(*itty).set_state(m_info.m_y + 1, state);
		}
	}
}

int32_t Mapper_front::next_object(Mapper_info& m_info)
{
	int32_t clear_steps = 0;

	// Location the path that we need.
	for (std::deque<Path>::iterator itty = m_info.m_map.begin();
			itty != m_info.m_map.end();
			++itty)
	{
		if((*itty).get_id() == m_info.m_x)
		{ 	// If the Path is found, based upon our current position, how many steps
			// can we move before hitting an object.
			int32_t y_adder = 1;
			while((*itty).get_state(m_info.m_y + y_adder) == STATE_CLEAR)
			{
				y_adder++;
				clear_steps++;
				printf("Return step");
				fflush(stdout);
			}
		}
	}
	return clear_steps;
}

void Mapper_front::move(Mapper_info& m_info, int32_t num_steps)
{
	// Just add the steps in the y position.
	m_info.m_y = m_info.m_y + num_steps;
}

