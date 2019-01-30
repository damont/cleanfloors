//////////////////////////////////////////////////////////////////////////////
/// @file map.cpp
///
/// @author David Monts
///
/// @date April 2016
///
/// @see map.h
//////////////////////////////////////////////////////////////////////////////

#include "map.h"
#include "mapper_front.h"
#include "mapper_right.h"
#include "mapper_back.h"
#include "mapper_left.h"

Map::Map() : m_heading(HEADING_FRONT),
			 m_mapper_info()
{
	m_mappers[HEADING_FRONT] = new Mapper_front();
	m_mappers[HEADING_RIGHT] = new Mapper_right();
	m_mappers[HEADING_BACK] = new Mapper_back();
	m_mappers[HEADING_LEFT] = new Mapper_left();
}

void Map::turn_right()
{
	m_heading++;
	if(m_heading == HEADING_COUNT)
	{
		m_heading = HEADING_FRONT;
	}
}

void Map::turn_left()
{
	m_heading--;
	if(m_heading < HEADING_FRONT)
	{
		m_heading = HEADING_COUNT - 1;
	}
}

void Map::set_location_to(int32_t x, int32_t y)
{
	m_mapper_info.m_x = x;
	m_mapper_info.m_y = y;
	m_heading = HEADING_FRONT;
}

void Map::go_to_location(int32_t x_dest, int32_t y_dest, Directions_list& directions)
{

	// We need to save off the starting x and y so we can make sure we set
	// our map back in the end.
	int32_t x_start = m_mapper_info.m_x;
	int32_t y_start = m_mapper_info.m_y;
	int32_t heading_start = m_heading;

	// First thing we need to do is see what direction we want to be going.
	int32_t x_to_travel = m_mapper_info.m_x - x_dest;
	int32_t y_to_travel = m_mapper_info.m_y - y_dest;

	int32_t next_distance = 0;

	while((m_mapper_info.m_x != x_dest) ||
		  (m_mapper_info.m_y != y_dest))
	{

		x_to_travel = m_mapper_info.m_x - x_dest;
		y_to_travel = m_mapper_info.m_y - y_dest;

		//////////////////////////////////////////////////////////////////////////
		// The first section handles traveling in the x direction
		//////////////////////////////////////////////////////////////////////////
		if(x_to_travel != 0)
		{
			if(x_to_travel > 0)
			{
				while(m_heading != HEADING_LEFT)
				{
					directions.push_back(LEFT_TURN);
					turn_left();
				}
			}
			else
			{
				while(m_heading != HEADING_RIGHT)
				{
					directions.push_back(RIGHT_TURN);
					turn_right();
				}
			}

			next_distance = next_object();
			if(next_distance <= std::abs(x_to_travel))
			{
				directions.push_back(next_distance);
				move(next_distance);
			}
			else
			{
				directions.push_back(std::abs(x_to_travel));
				move(std::abs(x_to_travel));
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// The second section handles traveling in the y direction
		//////////////////////////////////////////////////////////////////////////
		if(y_to_travel != 0)
		{
			if(y_to_travel > 0)
			{
				while(m_heading != HEADING_BACK)
				{
					directions.push_back(RIGHT_TURN);
					turn_right();
				}
			}
			else
			{
				while(m_heading != HEADING_FRONT)
				{
					directions.push_back(LEFT_TURN);
					turn_left();
				}
			}

			next_distance = next_object();
			if(next_distance <= std::abs(y_to_travel))
			{
				directions.push_back(next_distance);
				move(next_distance);
			}
			else
			{
				directions.push_back(std::abs(y_to_travel));
				move(std::abs(y_to_travel));
			}
		}
	}
	m_mapper_info.m_x = x_start;
	m_mapper_info.m_y = y_start;
	m_heading = heading_start;
}

void Map::clear_map()
{
	// Reset coordinates and heading
	m_mapper_info.m_x = 0;
	m_mapper_info.m_y = 0;
	m_heading = HEADING_FRONT;

	// Reset our map, add back the first row!! We depend upon this being there.
	m_mapper_info.m_map.clear();
	Path add_me(0, 0, STATE_CLEAR);
	m_mapper_info.m_map.push_back(add_me);
}

void Map::move_and_update(int32_t num_steps, State state)
{
	m_mappers[m_heading]->move_and_update(m_mapper_info, num_steps, state);
}

void Map::update_current(State state)
{
	m_mappers[m_heading]->update_current(m_mapper_info, state);
}

void Map::update_front(State state)
{
	m_mappers[m_heading]->update_front(m_mapper_info, state);
}

int32_t Map::next_object()
{
	return m_mappers[m_heading]->next_object(m_mapper_info);
}

void Map::move(int32_t num_steps)
{
	m_mappers[m_heading]->move(m_mapper_info, num_steps);
}
