/*
 * distance_process.h
 *
 *  Created on: Jul 16, 2015
 *      Author: David
 */

#ifndef INCLUDE_NAVIGATION_H_
#define INCLUDE_NAVIGATION_H_

#include <stdint.h>


////////////////////////////////////////////////////////////////////////////////////////
/// @class Navigation_interface
///
/// @brief Provides all functionality necessary to control the vehicle's movement
////////////////////////////////////////////////////////////////////////////////////////
class Navigation_interface
{
public:


	////////////////////////////////////////////////////////////////////////////////////
	/// @public init_hardware
	///
	///	@brief Initialize the hardware for the distance sensor and the two motor controller
	////////////////////////////////////////////////////////////////////////////////////
	static void init_hardware();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public get_distance_to_object
	///
	///	@brief Returns the distance to an object. Note that if the vehicle has just booted
	///		   or just made a turn then it will have to actually travel some amount of
	///		   distance in order to tell the distance.
	///
	/// @parm[out] distance - Returns the current distance to an object in cms.
	///
	/// @returns If the vehicle has to move any, returns that distance, else returns 0.
	////////////////////////////////////////////////////////////////////////////////////
	static uint32_t get_distance_to_object(uint32_t &distance);

	////////////////////////////////////////////////////////////////////////////////////
	/// @public turn_left
	///
	///	@brief Turns the vehicle approximately 90 degrees to the left.
	////////////////////////////////////////////////////////////////////////////////////
	static void turn_left();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public turn_right
	///
	///	@brief Turns the vehicle approximately 90 degrees to the right.
	////////////////////////////////////////////////////////////////////////////////////
	static void turn_right();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public move_until_hit
	///
	///	@brief Commands the vehicle to move until it hits an object
	///
	/// @returns Returns the distance the vehicle travels.
	////////////////////////////////////////////////////////////////////////////////////
	static uint32_t move_until_hit();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public try_to_move
	///
	///	@brief Commands the vehicle to move a certain distance if it can
	///
	/// @parm[in] distance_to_move - The distance the vehicle is trying to move
	///
	/// @returns Returns the distance the vehicle travels.
	////////////////////////////////////////////////////////////////////////////////////
	static uint32_t try_to_move(uint32_t distance_to_move);

	static uint32_t step_motors_forward(uint32_t);

	static uint32_t step_motors_backward(uint32_t);

	static void clear_motors();

	////////////////////////////////////////////////////////////////////////////////////
	/// @public Constructor
	///
	///	@brief Nothing to initialize
	////////////////////////////////////////////////////////////////////////////////////
	Navigation_interface() {};

private:
	// Currently the method for measuring distance we are using says that 2.5cm = 1inch.
	// We are going to have a constant for cm for one step of a motor. The 12V stepper
	// motor has a stepping accuracy of 200 steps per revolution, and the wheel has a
	// diameter of 4inches (10cm), calculate a circumference of 31.4cm (2*d). This means
	// that every step moves (31.4cm / 200) = .15cm
	static float const CM_PER_STEP = .15;

	// Number of voltage to distance ratios in the distance maps
	static uint32_t const MAP_SIZE = 12;

	// There is span in the return voltage for the distance sensor where above 2.5V will
	// yield a distance of 15cm to an object, however, this could be fairly inaccurate.
	static float const MAX_VOLTAGE_DISTANCE = 15;

	// Minimum distance the vehicle will get to an object
	static uint32_t const MINIMUM_DISTANCE_TO_OBJECT = 20;

	// Number of steps needed on both motors to make a 90 degree turn
	static uint32_t const STEPS_FOR_90_DEGREE_TURN = 130;

	// Distance to use in order to get a fairly precise measurement of how far we are
	// from an object
	static uint32_t const DISTANCE_FOR_VOLTAGE_READ = 5;

	static uint32_t const MOTOR_WAIT_TIME = 70000;


	////////////////////////////////////////////////////////////////////////////////////
	/// @struct Voltage_distance_t
	///
	///	@brief Structure to map the readings from the distance sensor to a distance
	////////////////////////////////////////////////////////////////////////////////////
	struct Voltage_distance_t
	{
		float m_voltage;
		uint32_t m_distance;
	};

	// Private member variables
	static bool s_initialized;
	static bool s_distance_known;
	static Voltage_distance_t s_voltage_increase_distance_map[MAP_SIZE];
	static Voltage_distance_t s_voltage_decrease_distance_map[MAP_SIZE];
	static uint32_t s_last_distance;


	// Private member functions
	static float get_sensor_voltage();
	static uint32_t get_distance_increasing(float voltage);
	static uint32_t get_distance_decreasing(float voltage);
	static uint32_t force_distance_read();



};



#endif /* INCLUDE_NAVIGATION_H_ */
