/* NUCLEO-F103 project setup
** by LE-NGUYEN ANH-TUAN
**       (WEEKEND LE)
**
** NOTICE: This project is setup for the STM Nucleo Board
**
**/

/**
** main.c
**/

#include "project.h"


/* Public variables */


/* Function prototype */


/* Main Function */
int main(void)
{
	vHardwareSetup();

	// Turn LED on
	GPIOA->BSRR = GPIO_BSRR_BS5;

	while(1)
	{
		GPIOA->BSRR = GPIO_BSRR_BS5;
		for(volatile uint32_t i = 0; i < 900000; i++)
		{
			__NOP();
		}
		GPIOA->BSRR = GPIO_BSRR_BR5;
		for(volatile uint32_t i = 0; i < 900000; i++)
		{
			__NOP();
		}
		__NOP();
	}

	return 0;
}
