/*
 * vHardwareSetup.c
 *
 *  Created on: May 25, 2026
 *      Author: LeNguyenAnhTuan
 */

#include "project.h"

#define TIMER2_FREQUENCY 1000000UL // 1us clock

// Public variables

// Function Prototype
static void prvHardwareSetupSystemClock(void);
static void prvHardwareSetupGPIO(void);
static void prvHardwareSetupPortAndPin(void);
static void prvHardwareSetupTimer(void);
static void prvHardwareSetupUART(void);
static void prvHardwareSetupInterrupt(void);

// vHardwareSetup
void vHardwareSetup(void)
{
	prvHardwareSetupSystemClock();
	prvHardwareSetupPortAndPin();
	prvHardwareSetupGPIO();
	prvHardwareSetupTimer();
	prvHardwareSetupUART();
	prvHardwareSetupInterrupt();
}

/*
 *	prvHardwareSetupSystemClock
*/
static void prvHardwareSetupSystemClock(void)
{
	/* Set FLASH latency */
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	/* The clock source for the main MCU is from the MCO pin from ST-LINK */
	/* Enable HSE oscillator */
	RCC->CR |= (1UL << 18UL);				// Bypass the HSE internal MCU oscillator
	RCC->CR |= (1UL << 16UL);				// HSE clock enable
	while( (RCC->CR & RCC_CR_HSERDY) == 0);	// Wait for HSE to ready

	/* Switch SYSCLK to HSE */
	RCC->CFGR &= ~RCC_CFGR_SW;				// Clear the SW bits field
	RCC->CFGR |= RCC_CFGR_SW_HSE;			// HSE selected as system clock for now
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE); // Wait till the SYSCLK is set to HSE

	/* Main PLL configure and enable */
	RCC->CR &= ~RCC_CR_PLLON;				// Disable the PLL clock before setting
	RCC->CFGR |= RCC_CFGR_PLLSRC;			// Select HSE as PLL entry
	RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;		// Divide HSE clock by 1
	RCC->CFGR &= ~RCC_CFGR_PLLMULL;			// Clear the PLLMULL bits field
	RCC->CFGR |= RCC_CFGR_PLLMULL9;			// Multiply the HSE clock by 9 (8 * 9 = 72 MHz)
	RCC->CR |= RCC_CR_PLLON;				// Enable the PLL clock
	while( (RCC->CR & RCC_CR_PLLRDY) == 0);	// Wait for the PLL clock to ready

	// Before switching SYSCLK to 72 MHz
	// need to set the prescale value  for AHB, APB1, APB clocks
	/* AHB clock prescaler  */
	RCC->CFGR &= ~RCC_CFGR_HPRE;			// Clear the HPRE bits field
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;		// Divide by 1

	/* APB1 clock prescaler  */
	RCC->CFGR &= ~RCC_CFGR_PPRE1;			// Clear the PPRE1 bits field
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;		// Divide by 2

	/* APB2 clock prescaler  */
	RCC->CFGR &= ~RCC_CFGR_PPRE2;			// Clear the PPRE2 bits field
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;		// Divide by 1

	/* Switch SYSCLK to PLL */
	RCC->CFGR &= ~RCC_CFGR_SW;				// Clear the SW bits field
	RCC->CFGR |= RCC_CFGR_SW_PLL;			// PLL selected as system clock for now
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // Wait till the SYSCLK is set to PLL

	/* SYSTEM CLOCK IS NOW 72 MHz */
	/* AHB CLOCK: 72 MHZ */
	/* APB1 CLOCK: 36 MHz */
	/* APB2 CLOCK: 72 MHz */
	SystemCoreClockUpdate();				// Update the SystemCoreClock variable in system_stm32f1xx.c
}

/*
 *	prvHardwareSetupPort
*/
static void prvHardwareSetupPortAndPin(void)
{
	// Enable all IO ports clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN
				  | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;
	// Wait for IO ports clock to stable
	volatile uint32_t temp = (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN
				   | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN);
	while((RCC->APB2ENR & (temp)) != temp);
	(void)temp;

	// Reset all IO ports
	RCC->APB2RSTR |= RCC_APB2RSTR_IOPARST | RCC_APB2RSTR_IOPBRST
				   | RCC_APB2RSTR_IOPCRST | RCC_APB2RSTR_IOPDRST | RCC_APB2RSTR_IOPERST;
	// Release all IO ports from reset
	RCC->APB2RSTR &= ~(RCC_APB2RSTR_IOPARST | RCC_APB2RSTR_IOPBRST
				     | RCC_APB2RSTR_IOPCRST | RCC_APB2RSTR_IOPDRST | RCC_APB2RSTR_IOPERST);


}

/*
 *	prvHardwareSetupGPIO
 *	Check Table 20, 21 in Chapter 9 - Reference Manual
*/
static void prvHardwareSetupGPIO(void)
{
	/* Set pin PA5 (port A, pin 5) as general output pin to drive LD2 on the Nucleo board */
	// Reset PA5 control fields
	GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
	// Configure pin PA5 as output with 2MHz drive strength
	GPIOA->CRL |= GPIO_CRL_MODE5_1;
	// Configure pin PA5 as General purpose output push-pull
	GPIOA->CRL &= ~(GPIO_CRL_CNF5);
	// Set output as low
	GPIOA->BSRR = GPIO_BSRR_BR5;

	/* Set pin PA2 as USART2_TX */
	// Reset PA2 control fields
	GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);
	// Configure PA2 as output with 50 MHz drive strength
	GPIOA->CRL |= GPIO_CRL_MODE2; // 11
	// Configure pin PA2 as Alternate function output push-pull
	// MAKE SURE THAT ONLY ONE PERIPHERAL IS CONNECTED TO THE PIN
	// In this case, USART2_TX
	GPIOA->CRL |= GPIO_CRL_CNF2_1; // 10


	/* Set pin PA3 as USART2_RX */
	// Reset PA3 control fields
	GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);
	// Configure PA3 as input mode
	GPIOA->CRL &= ~(GPIO_CRL_MODE3); // 00
	// Configure pin PA3 as Input with pull-up
	// MAKE SURE THAT ONLY ONE PERIPHERAL IS CONNECTED TO THE PIN
	// In this case, USART2_RX
	GPIOA->CRL |= GPIO_CRL_CNF3_1; // 10
	// Activate pull-up resistor
	GPIOA->ODR |= GPIO_ODR_ODR3; // 1
}

/*
 *	prvHardwareSetupTimer
*/
static void prvHardwareSetupTimer(void)
{
	// Configure TIMER2 for Debug console
	// Enable clock for TIMER2 -> Timer 2 clock input 72 MHz
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	__NOP();
	// Disable TIMER2 before config
	TIM2->CR1 &= ~TIM_CR1_CEN;
	// Set TIMER2 Frequency: 1 ms
	TIM2->PSC = 71U;	// Counter clock frequency: 72 MHz / (PSC + 1) = 1 MHz
	TIM2->ARR =	999U;	// Timer output frequency: 1000000 / (999 + 1) = 1000
	// Auto-reload preload -> ARR is only updated after a cycle to avoid glitch
	TIM2->CR1 |= TIM_CR1_ARPE;
	// Count Direction: count up
	TIM2->CR1 &= ~TIM_CR1_DIR;
	// One-pulse Mode DISABLED: Timer run forever
	TIM2->CR1 &= ~TIM_CR1_OPM;
	// Update Request Source: Only counter overflow generate an update interrupt
	TIM2->CR1 |= TIM_CR1_URS;
	// Update Disable: enable event generation
	TIM2->CR1 &= ~TIM_CR1_UDIS;

    // Force an update event to load PSC and ARR into active registers
    TIM2->EGR = TIM_EGR_UG;   // generates UEV, loads shadow registers
    TIM2->SR  &= ~TIM_SR_UIF; // clear the update interrupt flag

	// Enable interrupt generation on update
	TIM2->DIER = TIM_DIER_UIE;

	// Enable the timer by enable the counter
	// Can be done in main.c
	//TIM2->CR1 |= TIM_CR1_CEN;
}

/*
 *	prvHardwareSetupInterrupt
*/
static void prvHardwareSetupInterrupt(void)
{
    __disable_irq();
    __ISB();

    // Set the interrupt priority
    // This device support 4 bits of interrupt => 16 priority level
    // 0 - highest, 15 - lowest
    NVIC_SetPriority(TIM2_IRQn, 15U);

    // Enable the interrupt
    NVIC_EnableIRQ(TIM2_IRQn);	// Enable TIM2 interrupt

    __enable_irq();
}

/*
 *	prvHardwareSetupUART
*/
static void prvHardwareSetupUART( void )
{
	/*
	 *	Configure UART2
	 *	Pins PA2 (TX) and PA3 (RX) are routed to the ST-Link Virtual COM port,
	 *  which appears as a serial terminal on the PC.
	 */
}
