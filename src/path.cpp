//////////////////////////////////////////////////////////////////////////////
/// @file path.cpp
///
/// @author David Monts
///
/// @date April 2016
///
/// @see path.h
//////////////////////////////////////////////////////////////////////////////

#include "path.h"

Path::Path(int32_t xid, int32_t yid, State state) : m_id(xid)
{
	Step new_step(yid, state);
	m_steps.push_front(new_step);
}

Path& Path::operator=(Path &rhs)
{
	m_id = rhs.m_id;

	// Copy over all of the steps that are in the rhs's list
	for (std::list<Step>::iterator itty = rhs.m_steps.begin(); itty != rhs.m_steps.end(); ++itty)
	{
		m_steps.push_back((*itty));
	}

	return *this;
}

void Path::set_state(int32_t yid, State state)
{
	// Need to see if the current step_id is in the list.
	// If it is, then set the state.
	// If we do not have it, create everything between it and what we have

	if(yid < m_steps.front().get_id())
	{
		// add to beginning
		int32_t front_id = m_steps.front().get_id();
		Step new_step(front_id, STATE_UNKNOWN);

		// Loop keep adding steps
		do
		{
			new_step.set_id(--front_id);
			m_steps.push_front(new_step);
		} while(yid < front_id);

		// We will have already added the step, just set the state of the step
		// to the correct state
		m_steps.front().set_state(state);
	}
	else if(yid > m_steps.back().get_id())
	{
		// add to end
		int32_t end_id = m_steps.back().get_id();
		Step new_step(end_id, STATE_UNKNOWN);

		do
		{
			new_step.set_id(++end_id);
			m_steps.push_back(new_step);
		} while(yid > end_id);

		m_steps.back().set_state(state);

	}
	else
	{
		// loop through, find the step and set the state
		for (std::list<Step>::iterator itty = m_steps.begin(); itty != m_steps.end(); ++itty)
		{
			if ((*itty).get_id() == yid)
			{
				(*itty).set_state(state);
				break;
			}
		}
	}
}

State Path::get_state(int32_t yid)
{
	// loop through, find the step and return the state.
	for (std::list<Step>::iterator itty = m_steps.begin(); itty != m_steps.end(); ++itty)
	{
		if ((*itty).get_id() == yid)
		{
			return (*itty).get_state();
		}
	}

	// If we get here then we have not mapped the current location so return unknown
	return STATE_UNKNOWN;
}

int32_t Path::get_id()
{
	return m_id;
}

void Path::set_id(int32_t xid)
{
	m_id = xid;
}
