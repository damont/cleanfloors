/*
 * distance_process.cpp
 *
 *  Created on: Jul 16, 2015
 *      Author: David
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <cmath>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"

#include "navigation.h"
#include "easy_math.h"
#include "button_isr.h"
#include "bumper.h"


bool Navigation_interface::s_initialized = false;
bool Navigation_interface::s_distance_known = false;

uint32_t Navigation_interface::s_last_distance = 0;

// Constants for doing half steps.
static uint8_t const left_motor_half_3_2 = 0b00110000u;
static uint8_t const left_motor_half_2_1 = 0b01100000u;
static uint8_t const left_motor_half_1_0 = 0b11000000u;
static uint8_t const left_motor_half_0_3 = 0b10010000u;

static uint8_t const right_motor_half_3_2 = 0b00011000u;
static uint8_t const right_motor_half_2_1 = 0b00001100u;
static uint8_t const right_motor_half_1_0 = 0b00000110u;
static uint8_t const right_motor_half_0_3 = 0b00010010u;


// These arrays are arrays of voltage and distance (cm) pairs. The values were taken from the
// Sharp IR sensor technical data sheet.
Navigation_interface::Voltage_distance_t Navigation_interface::s_voltage_increase_distance_map[] = {{2.75, 15},
		{2.5, 20},
		{2.25, 25},
		{2, 30},
		{1.75, 35},
		{1.5, 40},
		{1.25, 50},
		{1, 64},
		{.75, 90},
		{.5, 130},
		{.25, 150},
		{0, 150}};
Navigation_interface::Voltage_distance_t Navigation_interface::s_voltage_decrease_distance_map[] = {{2.75, 15},
		{2.5, 12},
		{2.25, 10},
		{2, 10},
		{1.75, 8},
		{1.5, 6},
		{1.25, 5},
		{1, 4},
		{.75, 3},
		{.5, 2},
		{.25, 1},
		{0, 1}};

////////////////////////////////////////////////////////////////////////////////////
/// @public initialize
///
///	@brief Initialize the hardware to read the distance sensor
////////////////////////////////////////////////////////////////////////////////////
void Navigation_interface::init_hardware()
{
	if (s_initialized == false)
	{

		// Enable the ADC peripheral
		SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

		// Add in something with hardware averaging
		ADCHardwareOversampleConfigure(ADC0_BASE, 64);

		// Configure the senquencer for the ADC
		ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

		// Sample all four lines on sequencer 1
		ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7);
		ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH7);
		ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH7);
		ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);

		// Now enable sequencer 1
		ADCSequenceEnable(ADC0_BASE, 1);

		// Want to enable system peripheral for GPIO port E for the right-hand motor
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

		// Unlock the GPIO commit control register
		HWREG(GPIO_PORTE_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
		HWREG(GPIO_PORTE_BASE + GPIO_O_CR) = 0x01;
		HWREG(GPIO_PORTE_BASE + GPIO_O_LOCK) = 0;

		GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, GPIO_DIR_MODE_OUT);
		GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

		GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4);

		// Want to enable system peripheral for GPIO port C for the left-hand motor
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

		// Unlock the GPIO commit control register
		HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
		HWREG(GPIO_PORTC_BASE + GPIO_O_CR) = 0x01;
		HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK) = 0;

		GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_DIR_MODE_OUT);
		GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

		GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

		s_initialized = true;
	}

}

uint32_t Navigation_interface::get_distance_to_object(uint32_t &distance)
{

	uint32_t distance_traveled = 0;

	if(s_distance_known == false)
	{

		distance_traveled = force_distance_read();
	}

	// The value for the current distance will be placed in the s_last_distance variable
	distance = s_last_distance;

	return distance_traveled;
}

uint32_t Navigation_interface::force_distance_read()
{
	float first_voltage_reading = get_sensor_voltage();

	(void) step_motors_forward(DISTANCE_FOR_VOLTAGE_READ);

	// Now need to get the voltage reading again.
	float second_voltage_reading = get_sensor_voltage();

	uint32_t distance = 0;

	if (first_voltage_reading > 2.5)
	{
		// Landing here means that we are in the cross-over, and we need to be very careful
		distance = MAX_VOLTAGE_DISTANCE;
	}
	else
	{
		// If the object is super far away (> 100cms), we don't know what to do,
		// so just return a safe(ish) distance.
		float voltage_diff = second_voltage_reading - first_voltage_reading;
		if(std::abs(voltage_diff) < .15)
		{
			distance = 30;
		}
		else
		{
			if (second_voltage_reading > first_voltage_reading)
			{
				// Landing here means in the safe zone for getting voltages and definitely on
				// the up slope.
				distance = get_distance_increasing(second_voltage_reading);
			}
			else
			{
				distance = get_distance_decreasing(second_voltage_reading);
			}
		}
	}

	s_last_distance = distance;
	s_distance_known = true;

	return DISTANCE_FOR_VOLTAGE_READ;

}

////////////////////////////////////////////////////////////////////////////////////
/// @public execute
///
///	@brief Main process for the execution of the distance process
////////////////////////////////////////////////////////////////////////////////////
uint32_t Navigation_interface::step_motors_backward(uint32_t distance_cm)
{
	uint8_t left_motor_out = 128;
	uint8_t right_motor_out = 2;

	// Since every step moves about .157cm, then 4 steps moves .63cm which is about equal to
	// .6 cm. So in order to see how many times to go through the loop, do the distance times 10
	// and then divide by 6.
	int times_through_loop = (distance_cm * 10) / 6;


	for (int j = 0; j < times_through_loop; j++)
	{

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_1_0);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_1_0);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_2_1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_2_1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_3_2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_3_2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_0_3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_0_3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	}

//	// Since the first step really isn't a step, need to one more
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

//	// Now write all 0s so that we can start fresh next time
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	s_distance_known = false;

	return distance_cm;

}

uint32_t Navigation_interface::step_motors_forward(uint32_t distance_cm)
{
	uint8_t left_motor_out = 128;
	uint8_t right_motor_out = 2;

	float true_distance = 0.0;

	// Since every step moves about .157cm, then 4 steps moves .63cm which is about equal to
	// .6 cm. So in order to see how many times to go through the loop, do the distance times 10
	// and then divide by 6.
	int times_through_loop = (distance_cm * 10) / 6;


	for (int j = 0; (j < times_through_loop) && (is_obstacle_present() == NO_CONTACT); j++)
	{

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_3_2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_3_2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_2_1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_2_1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_1_0);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_1_0);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_0_3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_0_3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		true_distance += .63;

	}

//	// Since the first step really isn't a step, need to one more
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 3);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 3);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

//	// Now write all 0s so that we can start fresh next time
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	s_distance_known = false;

	return (uint32_t)true_distance;
}

float Navigation_interface::get_sensor_voltage()
{
	uint32_t analog_values[4];

	uint32_t average_analog;
	float distance_voltage;

	// clear the ADC flag
	ADCIntClear(ADC0_BASE, 1);

	// Trigger the ADC to start
	ADCProcessorTrigger(ADC0_BASE, 1);

	// Wait for the trigger to happen
	while(!ADCIntStatus(ADC0_BASE, 1, false))
	{
		// Spinning until the trigger occurs
	}

	// Get the values from the FIFO into our buffer
	ADCSequenceDataGet(ADC0_BASE, 1, analog_values);

	// Calculate the average of our four values
	average_analog = (analog_values[0] + analog_values[1] + analog_values[2] + analog_values[3] + 2)/4;

	// Get the voltage from the analog readings.
	distance_voltage = (3.3 * (float)average_analog) / 4096.0;
	//printf("%f\n", distance_voltage);
	//fflush(stdout);

	return distance_voltage;
}

void Navigation_interface::turn_left()
{
	// In order to turn left, both motors should move clockwise, however I do not know how much.
	uint8_t left_motor_out = 16;
	uint8_t right_motor_out = 2;

	uint32_t times_through_loop = STEPS_FOR_90_DEGREE_TURN / 4;

	for (uint32_t j = 0; j < times_through_loop; j++)
	{

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out << 3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_0_3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_0_3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_3_2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_1_0);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out << 1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_2_1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_2_1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out << 2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_1_0);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_3_2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);
	}

	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out << 3);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 3);
	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_0_3);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_0_3);
	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_2_1);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_2_1);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);
//
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out << 2);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out << 2);
//
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);
//
//	// Now write all 0s so that we can start fresh next time
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	s_distance_known = false;
}

void Navigation_interface::turn_right()
{

	// In order to turn right, both motors need to turn
	uint8_t left_motor_out = 128;
	uint8_t right_motor_out = 16;

	uint32_t times_through_loop = STEPS_FOR_90_DEGREE_TURN / 4;

	for (uint32_t j = 0; j < times_through_loop; j++)
	{
		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out >> 3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_0_3);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_0_3);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_1_0);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_3_2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out >> 1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_2_1);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_2_1);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out >> 2);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

		GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_3_2);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_1_0);
		for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	}

	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 3);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out >> 3);
	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_0_3);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_0_3);
	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_half_2_1);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_half_2_1);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);
//
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, left_motor_out >> 2);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, right_motor_out >> 2);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

//	// Now write all 0s so that we can start fresh next time
//	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0);
//	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);
//	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);

	s_distance_known = false;
}

uint32_t Navigation_interface::move_until_hit()
{

//	if (minimum_distance < MINIMUM_DISTANCE_TO_OBJECT)
//	{
//		minimum_distance = MINIMUM_DISTANCE_TO_OBJECT;
//	}
//
//	uint32_t current_distance = 0;
//
//	uint32_t distance_traveled = get_distance_to_object(current_distance);
//	printf("Sensor distance: %d\n", current_distance);
//	fflush(stdout);
//
//	while (current_distance > (minimum_distance + DISTANCE_FOR_VOLTAGE_READ))
//	{
//		distance_traveled += step_motors_forward((current_distance/2) - DISTANCE_FOR_VOLTAGE_READ);
//		distance_traveled += get_distance_to_object(current_distance);
//		printf("Sensor distance: %d\n", current_distance);
//		fflush(stdout);
//	}
//
//	s_distance_known = false;
//	printf("Total Distance Traveled: %d\n", distance_traveled);
//	fflush(stdout);
//	return distance_traveled;

	uint32_t distance_traveled = 0;

	while(is_obstacle_present() == NO_CONTACT)
	{
		// Trying to move forward by 3 will ensure that we get a fairly accurate value back
		// since the calculation travels (3*10)/6
		distance_traveled += step_motors_forward(3);
	}

	return distance_traveled;
}

uint32_t Navigation_interface::try_to_move(uint32_t distance_to_move)
{
//	uint32_t distance_to_object = 0;
//	uint32_t distance_traveled = get_distance_to_object(distance_to_object);
//
//	distance_to_move = distance_to_move - distance_traveled;
//
//	if(distance_to_move <= (distance_to_object - 10))
//	{
//		distance_traveled += step_motors_forward(distance_to_move);
//	}
//
//	return distance_traveled;

	uint32_t distance_traveled = 0;

	while((is_obstacle_present() == NO_CONTACT)
		  && (distance_traveled < distance_to_move))
	{
		// Trying to move forward by 3 will ensure that we get a fairly accurate value back
		// since the calculation travels (3*10)/6
		distance_traveled += step_motors_forward(3);
	}

	return distance_traveled;
}

uint32_t Navigation_interface::get_distance_increasing(float voltage)
{
	uint32_t return_distance = MAX_VOLTAGE_DISTANCE;

	uint32_t last_index = MAP_SIZE;
	uint32_t current_index = MAP_SIZE/2;
	uint32_t index_difference = 0;

	if (voltage < 3.0)
	{
		while (absolute_value(s_voltage_increase_distance_map[current_index].m_voltage, voltage) > .125)
		{
			index_difference = (absolute_value(current_index, last_index) / 2);
			last_index = current_index;

			if (voltage > s_voltage_increase_distance_map[current_index].m_voltage)
			{
				// The array is structured so that the lower indices have the higher voltage values,
				// so since need a larger voltage value, move down in the array
				if (index_difference > 0)
				{
					current_index -= index_difference;
				}
				else
				{
					current_index -= 1;
				}
			}
			else
			{
				// The array is structured so that the lower indices have the higher voltage values,
				// so since need a smaller voltage value, move up in the array
				if (index_difference > 0)
				{
					current_index += index_difference;
				}
				else
				{
					current_index += 1;
				}
			}
		}

		return_distance = s_voltage_increase_distance_map[current_index].m_distance;
	}

	s_last_distance = return_distance;
	return return_distance;

}

uint32_t Navigation_interface::get_distance_decreasing(float voltage)
{
	uint32_t return_distance = MAX_VOLTAGE_DISTANCE;

	uint32_t last_index = MAP_SIZE;
	uint32_t current_index = MAP_SIZE/2;
	uint32_t index_difference = 0;

	if (voltage < 3.0)
	{
		while (absolute_value(s_voltage_decrease_distance_map[current_index].m_voltage, voltage) > .125)
		{
			index_difference = (absolute_value(current_index, last_index) / 2);
			last_index = current_index;

			if (voltage > s_voltage_decrease_distance_map[current_index].m_voltage)
			{
				// The array is structured so that the lower indices have the higher voltage values,
				// so since need a larger voltage value, move down in the array
				if (index_difference > 0)
				{
					current_index -= index_difference;
				}
				else
				{
					current_index -= 1;
				}
			}
			else
			{
				// The array is structured so that the lower indices have the higher voltage values,
				// so since need a smaller voltage value, move up in the array
				if (index_difference > 0)
				{
					current_index += index_difference;
				}
				else
				{
					current_index += 1;
				}
			}
		}

		return_distance = s_voltage_decrease_distance_map[current_index].m_distance;
	}

	s_last_distance = return_distance;
	return return_distance;
}

void Navigation_interface::clear_motors()
{

	// Rest the motors so we don't accidentally over heat something.
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, 0);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);
	for (uint32_t i = 0; i < MOTOR_WAIT_TIME; i++);
}

