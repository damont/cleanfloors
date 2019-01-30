//////////////////////////////////////////////////////////////////////////////
/// @file step.cpp
///
/// @author David Monts
///
/// @date April 2016
///
/// @see step.h
//////////////////////////////////////////////////////////////////////////////

#include "step.h"

Step::Step(int32_t id, State state) : m_id(id), m_state(state) {}

Step& Step::operator=(Step &rhs)
{
	m_id = rhs.m_id;
	m_state = rhs.m_state;

	return *this;
}

void Step::set_id(int32_t id)
{
	m_id = id;
}

int32_t Step::get_id()
{
	return m_id;
}

void Step::set_state(State state)
{
	m_state = state;
}

State Step::get_state()
{
	return m_state;
}
