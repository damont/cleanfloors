/*
 * bumper.h
 *
 *  Created on: Jun 4, 2016
 *      Author: David
 */

#ifndef INCLUDE_BUMPER_H_
#define INCLUDE_BUMPER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	CONTACT = 0,
	NO_CONTACT = 1
};

void enable_bumper();
void handle_bumper();
int32_t is_obstacle_present();
void clear_obstacle_reading();


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_BUMPER_H_ */
