//////////////////////////////////////////////////////////////////////////////
/// @file step.h
///
/// @author David Monts
///
/// @date April 2016
///
/// @brief A step is the lowest level of mapping. It provides the y coordinate
///		   for the map and a description of the contents at the location. It
///		   depends upon a higher level to manage the x coordinate.
//////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_STEP_H_
#define INCLUDE_STEP_H_


#include <stdint.h>
#include <state.h>

//////////////////////////////////////////////////////////////////////////////
/// @class Step
///
/// @brief Provides the steps moving north and south.
//////////////////////////////////////////////////////////////////////////////
class Step
{

public:

	//////////////////////////////////////////////////////////////////////////
	/// @constructor
	///
	/// @brief Creates a new Step object and sets the id and state
	///
	/// @param[in] id - The coordinate for the step
	/// @param[in] state - Details what is at this location
	//////////////////////////////////////////////////////////////////////////
	Step(int32_t id, State state);

	//////////////////////////////////////////////////////////////////////////
	/// @public
	///
	/// @brief Assignment operator
	///
	/// @param[in] rhs - The Step object that is going to be copied
	//////////////////////////////////////////////////////////////////////////
	Step& operator=(Step &rhs);

	//////////////////////////////////////////////////////////////////////////
	/// @public set_id
	///
	/// @brief Sets the y coordinate for the current location
	///
	/// @param[in] id - the y coordinate
	//////////////////////////////////////////////////////////////////////////
	void set_id(int32_t id);

	//////////////////////////////////////////////////////////////////////////
	/// @public get_id
	///
	/// @brief Gets the y coordinate for the current location
	///
	/// @returns - the y coordinate
	//////////////////////////////////////////////////////////////////////////
	int32_t get_id();


	//////////////////////////////////////////////////////////////////////////
	/// @public set_state
	///
	/// @brief Sets the state for the current location
	///
	/// @param[in] state - Describes the contents of the current location
	//////////////////////////////////////////////////////////////////////////
	void set_state(State state);

	//////////////////////////////////////////////////////////////////////////
	/// @public get_state
	///
	/// @brief Gets the state for the current location
	///
	/// @returns A description of the content of the current location
	//////////////////////////////////////////////////////////////////////////
	State get_state();

private:
	int32_t m_id;	// y coordinate for the location
	State m_state;  // Determines whether the current location has an object
					// or if it has been mapped
};


#endif /* INCLUDE_STEP_H_ */