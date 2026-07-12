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
static volatile uint64_t ullSecond = 0UL;

/* Function prototype */


/* Main Function */
int main(void)
{
	// Disable global interrupt
	__disable_irq();
	__ISB();

	vHardwareSetup();

	// Turn LED on
	GPIOA->BSRR = GPIO_BSRR_BS5;

	// Turn TIMER2 ON
	TIM2->CR1 |= TIM_CR1_CEN;

	// Turn USART2 ON
	USART2->CR1 |= USART_CR1_UE;

	// Enable global interrupt
	__enable_irq();

	vMsgToSend("Hello World!\r\n");
	vPrintWelcomeBanner();

	while(1)
	{
        /* Main loop processing */

        char cRxBuffer[64];
        if (xReadUSART(cRxBuffer, 64))
        {
            if (cRxBuffer[0] != '\0')   // Only print non-empty lines
            {
                vMsgToSend("Received: ");
                vMsgToSend(cRxBuffer);
                vMsgToSend("\r\n");
            }
        }

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
		//GPIOA->BSRR = GPIO_BSRR_BS5; // Turn LED ON
	}
	if( ulTimer2Counter > 1000UL )
	{
		ulTimer2Counter = 0UL;
		//GPIOA->BSRR = GPIO_BSRR_BR5; // Turn LED OFF
		vMsgToSend("Timer Loop Count: ");
		vUint64ToString(ullSecond, cUint64String);
		vMsgToSend(cUint64String);
		vMsgToSend("\r\n");
		ullSecond = ullSecond + 1ULL;
	}

	// Clear TIMER2 Interrupt Flag
	TIM2->SR = 0x0000U;
}

/*
 *	EXTI13 ISR:
 *	React to button press (falling edge) event
 */
void EXTI15_10_IRQHandler( void )
{
	/* Verify that EXTI Line 13 triggered the ISR as this interrupt is shared by Line 10 - 15 */
	if( (EXTI->PR & EXTI_PR_PR13) != 0UL )
	{


		// Read PC13 state - 0 means pressed
		if( (GPIOC->IDR & GPIO_IDR_IDR13) == 0U )
		{
			static uint8_t ucLedState = 0U; // Check LED State
			if( ucLedState == 0U )
			{
				GPIOA->BSRR = GPIO_BSRR_BS5; // Turn LED ON
				ucLedState = 1U;
				vMsgToSend("Button pressed: LED ON\r\n");
			}
			else
			{
				GPIOA->BSRR = GPIO_BSRR_BR5; // Turn LED OFF
				ucLedState = 0U;
				vMsgToSend("Button pressed: LED OFF\r\n");
			}
		}
		/* Clear EXTI Line 13 ISR */
		EXTI->PR = EXTI_PR_PR13;
	}

}
