/*
 * easy_math.cpp
 *
 *  Created on: Aug 8, 2015
 *      Author: David
 */

#include "easy_math.h"

float absolute_value(float first, float second)
{
	if (first > second)
	{
		return (first - second);
	}
	else
	{
		return (second - first);
	}
}

uint32_t absolute_valute(uint32_t first, uint32_t second)
{
	if (first > second)
	{
		return (first - second);
	}
	else
	{
		return (second - first);
	}
}


void sleeperson(uint32_t sleepy_time)
{
	for (uint32_t i = 0; i < sleepy_time; i++)
	{

	}
}


