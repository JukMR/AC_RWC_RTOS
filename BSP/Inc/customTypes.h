/*
 * customTypes.h
 *
 *  Created on: Jul 25, 2021
 *      Author: julian
 */

#ifndef INC_CUSTOMTYPES_H_
#define INC_CUSTOMTYPES_H_


typedef struct {
	char command[16];
	char arg1[16];
	char arg2[16];
	uint32_t time;
} xScheduledTask_t;

#endif /* INC_CUSTOMTYPES_H_ */
