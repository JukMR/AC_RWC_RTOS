/*
 * customTypes.h
 *
 *  Created on: Jul 25, 2021
 *      Author: julian
 */

#ifndef INC_CUSTOMTYPES_H_
#define INC_CUSTOMTYPES_H_

#include <stdint.h>

typedef struct {
	char pcCommand[16];
	char pcArg1[16];
	char pcArg2[16];
	uint32_t uTime;
} xDelayTask_t;

#endif /* INC_CUSTOMTYPES_H_ */
