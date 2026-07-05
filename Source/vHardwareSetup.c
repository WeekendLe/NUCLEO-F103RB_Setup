/*
 * vHardwareSetup.c
 *
 *  Created on: May 25, 2026
 *      Author: LeNguyenAnhTuan
 */

#include "project.h"

// Public variables


// Function Prototype
static void prvHardwareSetupSystemClock(void);
static void prvHardwareSetupGPIO(void);
static void prvHardwareSetupPortAndPin(void);


// vHardwareSetup
void vHardwareSetup(void)
{
	prvHardwareSetupSystemClock();
	prvHardwareSetupPortAndPin();
	prvHardwareSetupGPIO();

}

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

	SystemCoreClockUpdate();				// Update the SystemCoreClock variable in system_stm32f1xx.c
}

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

static void prvHardwareSetupGPIO(void)
{
	/* Set pin PA5 (port A, pin 5) as general output pin to drive LD2 on the Nucleo board */
	// Reset the PA5 control fields
	GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5);
	// Configure pin PA5 as output with 2MHz drive strength
	GPIOA->CRL |= GPIO_CRL_MODE5_1;
	// Configure pin PA5 as General purpose output push-pull
	GPIOA->CRL &= ~(GPIO_CRL_CNF5);
	// Set output as low
	GPIOA->BSRR = GPIO_BSRR_BR5;
}
