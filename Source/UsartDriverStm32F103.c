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

/*
 *	USART2 Interrupt Handler
 */
void USART2_IRQHandler( void )
{
	/*
	 *  TX INTERRUPT
	 */
	// Check for status flag for TX Interrupt
	// AND check if TX Interrupt is enabled
	if((USART2->SR & USART_SR_TXE) && (USART2->CR1 & USART_CR1_TXEIE))
	    {
	        vWriteUSART();
	    }
	// According to the Manual, INT Flag for USART is cleared by hardware

	/*
	 *  RX INTERRUPT
	 */
	// Check for status flag for RX Interrupt
	// AND check if RX Interrupt is enabled
	if((USART2->SR & USART_SR_RXNE) && (USART2->CR1 & USART_CR1_RXNEIE))
	{
		// Read the received character from USART2->DR (RXNE flag clear automatically)
		unsigned char ucRxChar = (unsigned char)(USART2->DR & USART_DR_DR);

		// Write the received character into the RX Ring buffer
		uint16_t usNextHead = usRxHead + 1U;
		if( usNextHead >= RX_BUFFER_SIZE )
		{
			usNextHead = 0U;
		}
		if( usNextHead != usRxTail )
		{
			ucRxBuffer[usRxHead] = ucRxChar;
			usRxHead = usNextHead;
		}

		// Echo back logic start
		USART2->CR1 &= ~(USART_CR1_TXEIE | USART_CR1_RXNEIE);

		uint16_t usNextTxHead = usTxHead + 1U;
		if( usNextTxHead > (TX_BUFFER_SIZE - 1U) )
		{
			usNextTxHead = 0U;
		}

		if( usNextTxHead != usTxTail )
		{
			ucTxBuffer[usTxHead] = ucRxChar;
			usTxHead = usNextTxHead;
		}
		// else: buffer full → echo dropped silently
		// Echo back logic end
		USART2->CR1 |= (USART_CR1_TXEIE | USART_CR1_RXNEIE);
	}
}

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

// xReadUSART
bool xReadUSART(char * const pcBuffer, uint16_t usBufferSize)
{

	// Check if buffer is valid
	if( (usBufferSize == 0) || (pcBuffer == NULL) )
	{
		return false;
	}

	/* CRITICAL SECTION START: Disable RX interrupt to prevent the ISR from modifying Rx Ring Buffer */
	USART2->CR1 &= ~(USART_CR1_RXNEIE);

	// If the Rx Ring Buffer is empty -> Nothing to process
	if( usRxTail == usRxHead )
	{
		/* CRITICAL SECTION END: Re-enable RX Interrupt */
		USART2->CR1 |= USART_CR1_RXNEIE;
		return false;
	}

    /*
     * First pass:
     * 	- Scan the RX buffer from usRxTail to usRxHead to find line terminator (\r or \n) or (\r\n or \n\r)
     * 	- Measure the length of the message (excluding the terminator(s)).
     */
	uint16_t usIdx = usRxTail;          // Start scanning from the oldest byte
	uint16_t usMsgLen = 0U;             // Length of the message (no terminators)
	uint16_t usTerminatorLen = 0U;      // Length of the terminator (1 or 2)

	while( usIdx != usRxHead )
	{
		unsigned char ucCh = ucRxBuffer[usIdx];

		/* Check for a terminator character */
		if ((ucCh == (unsigned char)'\r') || (ucCh == (unsigned char)'\n'))
		{
			usTerminatorLen = 1U;

			uint16_t usNextIdx = usIdx + 1U;
			if( usNextIdx > (RX_BUFFER_SIZE - 1U) )
			{
				usNextIdx = 0;
			}

			if( usNextIdx != usRxHead )
			{
				unsigned char ucNextCh = ucRxBuffer[usNextIdx];
				// Check \r\n or \n\r
				if (((ucCh == (unsigned char)'\r') && (ucNextCh == (unsigned char)'\n')) ||
						((ucCh == (unsigned char)'\n') && (ucNextCh == (unsigned char)'\r')))
				{
					usTerminatorLen = 2U;
				}
			}
			break; // Terminator all found (one valid message)
		}
		/* If reach here -> count as a message */
		usMsgLen = usMsgLen + 1U;
		usIdx = usIdx + 1U;

        if( usIdx > (RX_BUFFER_SIZE - 1U) )	// Wrap around to check
        {
            usIdx = 0U;
        }
	}

	// No terminator found in Rx Ring Buffer
	// Message is incomplete -> return false
	if( usTerminatorLen == 0U )
	{
		/* CRITICAL SECTION END: Re-enable RX Interrupt */
		USART2->CR1 |= USART_CR1_RXNEIE;
		return false;
	}

	// If the receiving line larger than provided buffer -> flush the usRxTail to dismiss this cmd
	if( usMsgLen >= usBufferSize )
	{
		usRxTail = usRxTail + usMsgLen + usTerminatorLen;

		if( usRxTail > (RX_BUFFER_SIZE - 1) )
		{
			usRxTail = usRxTail % RX_BUFFER_SIZE;
		}

		/* CRITICAL SECTION END: Re-enable RX Interrupt */
		USART2->CR1 |= USART_CR1_RXNEIE;
		return false;
	}

	/*
	 * Second pass: copy the msg from the RX Ring Buffer in to the provided buffer
	 */
	uint16_t usCopyCount = usMsgLen;
	uint16_t usLastIdx = 0U;

	while( usCopyCount > 0U )
	{
		pcBuffer[usLastIdx] = (unsigned char)ucRxBuffer[usRxTail];

		// Advance tail circularly
		usRxTail = usRxTail + 1U;
		if( usRxTail > (RX_BUFFER_SIZE - 1U) )
		{
			usRxTail = 0U;
		}

		usLastIdx = usLastIdx + 1U;
		usCopyCount = usCopyCount - 1U;
	}

	pcBuffer[usLastIdx] = (unsigned char)'\0';

	// Skip the terminator character in the Rx Ring Buffer (prep for next read)
	uint16_t i = 0U;
	for( i = 0U; i < usTerminatorLen; i++ )
	{
		usRxTail = usRxTail + 1U;
		if( usRxTail > (RX_BUFFER_SIZE - 1U) )
		{
			usRxTail = 0U;
		}
	}

	/* CRITICAL SECTION END: Re-enable RX Interrupt */
	USART2->CR1 |= USART_CR1_RXNEIE;

	return true;
}

/*
 *  Print Welcome Banner
 */
void vPrintWelcomeBanner( void )
{
	vMsgToSend("===========================================\r\n");
	vMsgToSend("         NUCLEO-F103 SETUP PROJECT\r\n");
	vMsgToSend("===========================================\r\n");
	vMsgToSend(" [AUTHOR]: LE-NGUYEN ANH-TUAN (WEEKEND LE)\r\n");
	vMsgToSend(" [DATE]: 11 JULY 2026\r\n");
	while (usTxHead != usTxTail) { }
	while (!(USART2->SR & USART_SR_TC)) { }
	vMsgToSend("=====================V=====================\r\n");
	vMsgToSend("                   /\\_/\\                 \r\n");
	vMsgToSend("            -WL-  ( o.o ) -WL-             \r\n");
	vMsgToSend("                   > ^ <                   \r\n");
	while (usTxHead != usTxTail) { }
	while (!(USART2->SR & USART_SR_TC)) { }
}
