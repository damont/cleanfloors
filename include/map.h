//////////////////////////////////////////////////////////////////////////////
/// @file map.h
///
/// @author David Monts
///
/// @date April 2016
///
/// @brief The level of mapping that the application will actually interact with.
///		   The map depends upon the user to update its location!!!
//////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_MAP_H_
#define INCLUDE_MAP_H_

#include "mapper.h"
#include <vector>

typedef std::vector<int32_t> Directions_list;

enum
{
	RIGHT_TURN = -2,
	LEFT_TURN = -1
};

class Map
{
public:

	Map();

	void turn_right();
	void turn_left();
	void set_location_to(int32_t x, int32_t y);
	void go_to_location(int32_t x_dest, int32_t y_dest, Directions_list& directions);
	void clear_map();

	void move_and_update(int32_t num_steps, State state);
	void update_current(State state);
	void update_front(State state);

	int32_t next_object();
	void move(int32_t num_steps);

private:

	enum
	{
		HEADING_FRONT = 0,
		HEADING_RIGHT,
		HEADING_BACK,
		HEADING_LEFT,
		HEADING_COUNT
	};

	Mapper_info m_mapper_info;
	int32_t m_heading;

	Mapper * m_mappers[HEADING_COUNT];
};

#endif /* INCLUDE_MAP_H_ */
