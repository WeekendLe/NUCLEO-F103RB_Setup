/* NUCLEO-F103 project setup
** by LE-NGUYEN ANH-TUAN
**       (WEEKEND LE)
**
** Date: 2026-07-04
**
** NOTICE: This project is setup for the STM Nucleo-F103RB Board
**
**/

/**
** main.c
**/

#include "project.h"


/* Public variables */
static volatile uint32_t ulTimer2Counter = 0UL;


/* Function prototype */


/* Main Function */
int main(void)
{
	vHardwareSetup();

	// Turn LED on
	GPIOA->BSRR = GPIO_BSRR_BS5;

	// Turn TIMER2 ON
	TIM2->CR1 |= TIM_CR1_CEN;

	while(1)
	{

	}

	return 0;
}

/*
 *	TIMER2 Interrupt Handler
 */
void TIM2_IRQHandler( void )
{
	ulTimer2Counter = ulTimer2Counter + 1UL;

	if( ulTimer2Counter == 500UL )
	{
		GPIOA->BSRR = GPIO_BSRR_BS5; // Turn LED ON
	}
	if( ulTimer2Counter == 1000UL )
	{
		GPIOA->BSRR = GPIO_BSRR_BR5; // Turn LED OFF
		ulTimer2Counter = 0UL;
	}

	// Clear TIMER2 Interrupt Flag
	TIM2->SR = 0x0000U;
}
