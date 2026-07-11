/*
 * project.h
 *
 *  Created on: May 23, 2026
 *      Author: LeNguyenAnhTuan
 */

#ifndef PROJECT_H_
#define PROJECT_H_

// C library
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// STM32F103RBT6 CMSIS Header
#include "stm32f1xx.h"

// Device library

// Custom drivers
#include "UsartDriverStm32F103.h"

// Function Prototype
void vHardwareSetup(void);

#endif /* PROJECT_H_ */
