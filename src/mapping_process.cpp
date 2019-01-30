/*
 * mapping_process.cpp
 *
 *  Created on: Aug 4, 2015
 *      Author: David
 */

#include <stdio.h>
#include "mapping_process.h"
#include "navigation.h"
#include "button_isr.h"
#include "bumper.h"

int32_t const STEP_DISTANCE = 25;

void Mapping_process::initialize()
{
	//uint32_t current_distance;
	//uint32_t distance_traveled;

//	// Want to first see how far we are from the object
//	Navigation_interface::get_distance_to_object(current_distance);
//
//	printf("%d\n", current_distance);
//	fflush(stdout);
//
//	Navigation_interface::turn_right();
//
//	// Want to first see how far we are from the object
//	Navigation_interface::get_distance_to_object(current_distance);
//
//	printf("%d\n", current_distance);
//	fflush(stdout);
//
//	Navigation_interface::turn_left();
//
//	// Want to first see how far we are from the object
//	Navigation_interface::get_distance_to_object(current_distance);
//
//	printf("%d\n", current_distance);
//	fflush(stdout);

	//distance_traveled = Navigation_interface::move_until(15);
	//Navigation_interface::turn_right();
}

void Mapping_process::execute()
{
	m_map.clear_map();

	wait_on_push();

	clear_obstacle_reading();
	create_map();

	wait_on_push();
	// OK, let's try and map it out!!!
	Directions_list dirs;
	m_map.go_to_location(0, 0, dirs);
	navigate(dirs);

	Navigation_interface::clear_motors();
}


void Mapping_process::navigate(Directions_list directions)
{

	for (Directions_list::iterator itty = directions.begin();
		 itty != directions.end();
		 ++itty)
	{
		switch((*itty))
		{
		case RIGHT_TURN:
		{
			Navigation_interface::turn_right();
			m_map.turn_right();
			break;
		}

		case LEFT_TURN:
		{
			Navigation_interface::turn_left();
			m_map.turn_left();
			break;
		}

		default:
		{
			Navigation_interface::step_motors_forward((*itty) * STEP_DISTANCE);
			m_map.move((*itty) * STEP_DISTANCE);
			break;
		}
		}
	}
}

void Mapping_process::create_map()
{
	bool more_to_map = true;

	while(more_to_map)
	{
		move_forward();
		turn_right();
		more_to_map = move_across();


		if(more_to_map)
		{
			turn_right();
			move_forward();
			turn_left();
			more_to_map = move_across();
			turn_left();
		}
	}

}

bool Mapping_process::move_forward()
{
	int32_t cms = Navigation_interface::move_until_hit();

	if (cms > 8)
	{
		cms -= 8;
	}
	Navigation_interface::step_motors_backward(8);

	int32_t steps = cms / STEP_DISTANCE;
	int32_t left_over = cms % STEP_DISTANCE;
	Navigation_interface::step_motors_backward(left_over);
	clear_obstacle_reading();

	m_map.move_and_update(steps, STATE_CLEAR);
	m_map.update_front(STATE_OBJECT);

	// Only want to return true if we were able to move atleast 1 step.
	if(steps > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Mapping_process::move_across()
{
	int32_t cms = Navigation_interface::try_to_move(STEP_DISTANCE);

	if(cms >= STEP_DISTANCE)
	{
		m_map.move_and_update(1, STATE_CLEAR);
		return true;
	}
	else
	{
		Navigation_interface::step_motors_backward(cms);
		clear_obstacle_reading();
		m_map.update_front(STATE_OBJECT);
		return false;
	}
}

void Mapping_process::turn_right()
{
	Navigation_interface::turn_right();
	m_map.turn_right();
}

void Mapping_process::turn_left()
{
	Navigation_interface::turn_left();
	m_map.turn_left();
}
