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

// STM32F103RBT6 CMSIS Header
#include "stm32f1xx.h"

// Device library

// Function Prototype
void vHardwareSetup(void);

// ISR Prototype
void TIM2_IRQHandler(void);
void USART2_IRQHandler(void);

#endif /* PROJECT_H_ */
