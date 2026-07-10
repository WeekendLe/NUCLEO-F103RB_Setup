/*
 * UsartDriverStm32F103.c
 *
 *  Created on: Jul 10, 2026
 *      Author: Anh-Tuan Le-Nguyen
 */

#include "project.h"
#include "UsartDriverStm32F103.h"

// Transmit Software Ring Buffer
static volatile unsigned char ucTxBuffer[TX_BUFFER_SIZE];
static volatile uint16_t usTxHead = 0U;
static volatile uint16_t usTxTail = 0U;

// Receive Software Ring Buffer
static volatile unsigned char ucRxBuffer[RX_BUFFER_SIZE];
static volatile uint16_t usRxHead = 0U;
static volatile uint16_t usRxTail = 0U;

// Global arrays/buffers to store the converted string
char cUint16String[UINT16_TO_STRING_SIZE];
char cInt16String[INT16_TO_STRING_SIZE];
char cUint32String[UINT32_TO_STRING_SIZE];
char cInt32String[INT32_TO_STRING_SIZE];
char cUint64String[UINT64_TO_STRING_SIZE];
char cInt64String[INT64_TO_STRING_SIZE];
char cFractionalString[FRACTIONAL_TO_STRING_SIZE];

// vWriteUSART()
// Drop bytes from software TX ring buffer to DR
void vWriteUSART( void )
{
	/*
	** CRITICAL SECTION START: Temporary disable RX interrupt
	** Reason: The RX Interrupt that is function as echo back will write byte to software TX buffer,
	** that also modified the usTxHead and usTxTail
	*/
	USART2->CR1 &= ~(USART_CR1_RXNEIE); // Disable receive interrupt

	// Check if the the software buffer is not empty
	if( usTxHead != usTxTail )
	{
		/*
		** Feed bytes from software TX ring buffer to hardware DR
		*/
		USART2->DR = (uint32_t)(ucTxBuffer[usTxTail] & USART_DR_DR);

		// Advance tail pointer circularly
    	uint16_t usNextTxTail = usTxTail + 1U;
    	if( usNextTxTail > (TX_BUFFER_SIZE - 1U) )
    	{
       		usNextTxTail = 0U;
    	}
    	usTxTail = usNextTxTail;
	}
	else // usTxHead = usTxtail: Ring Buffer is empty
	{
		/* Disable the transmit interrupt as the UASRT_DR will be empty */
		USART2->CR1 &= ~(USART_CR1_TXEIE);
	}

	/* CRITICAL SECTION END: Restore RX interrupt */
	USART2->CR1 |= USART_CR1_RXNEIE; // Enable receive interrupt
}

// Message to send to the Software TX Ring Buffer
// Basically what the string to send
// Example: vMsgToSend("string to send");
void vMsgToSend( const char * const pcMsg )
{

	uint16_t usIdx = 0U;

    /* CRITICAL SECTION START: Disable both USART TX and RX interrupts */
    USART2->CR1 &= ~(USART_CR1_TXEIE | USART_CR1_RXNEIE);

	while( pcMsg[usIdx] != '\0' )
	{
		uint16_t usNextTxHead = usTxHead + 1U;	// Calculate the next slot in the Ring Buffer
		if( usNextTxHead > (TX_BUFFER_SIZE - 1U) ) // Wrap around for Ring Buffer Implementation
		{
			usNextTxHead = 0U;
		}
		/*
	     * Check for software buffer overflow.
	     * If the next head would equal the tail, the buffer is full and
	     * the byte is silently dropped – this ensures the function never blocks
	     * (fire-and-forget behavior) => Risk of overflow data if sending to fast
	     */
		if( usNextTxHead != usTxTail )
		{
			ucTxBuffer[usTxHead] = pcMsg[usIdx];	// write to current slot
			usTxHead = usNextTxHead;	// Move the pointer to next slot
		}

		usIdx = usIdx + 1U;
	}

    /* CRITICAL SECTION END: Re-enable both interrupts.
     * - TXEIE: starts transmission (ISR will fire if TXE is set)
     * - RXNEIE: restores the echo functionality */
    USART2->CR1 |= (USART_CR1_TXEIE | USART_CR1_RXNEIE);
}

