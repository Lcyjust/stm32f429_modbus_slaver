/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"
#include "usart.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "../include/mb.h"
#include "../include/mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{

	int i =0;
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     *
     */if(xRxEnable)
     {
 		//
 		// 如果使用了485控制芯片,那么在此处将485设置为接收模式
 		//

 		// MAX485_SetMode(MODE_RECV);

         __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);		// 使能接收非空中断
     }
     else
     {
         __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);		// 禁能接收非空中断
     }

     if(xTxEnable)
     {
 		//
 		// 如果使用了485控制芯片,那么在此处将485设置为发送模式
 		//

 		// MAX485_SetMode(MODE_SENT);

         __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);			// 使能发送为空中断
     }
     else
     {
         __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);		// 禁能发送为空中断
     }

}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = ulBaudRate;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;

	switch(eParity)
	{
	// 奇校验
	case MB_PAR_ODD:
		huart1.Init.Parity = UART_PARITY_ODD;
		huart1.Init.WordLength = UART_WORDLENGTH_9B;			// 带奇偶校验数据位为9bits
		break;

	// 偶校验
	case MB_PAR_EVEN:
		huart1.Init.Parity = UART_PARITY_EVEN;
		huart1.Init.WordLength = UART_WORDLENGTH_9B;			// 带奇偶校验数据位为9bits
		break;

	// 无校验
	default:
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;			// 无奇偶校验数据位为8bits
		break;
	}
	return HAL_UART_Init(&huart1) == HAL_OK ? TRUE : FALSE;

}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	USART1->DR = ucByte;
	return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
	*pucByte = (USART1->DR & (uint16_t)0x00FF);
	return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void USART1_IRQHandler(void)
{
//#ifdef MB_RTU_ENABLED
	if(__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE)!= RESET)
		{
			prvvUARTRxISR();//接收中断
		}

		if(__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE)!= RESET)
		{
			prvvUARTTxReadyISR();//发送缓冲区空中断
		}


	  HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
	  HAL_UART_IRQHandler(&huart1);


//#ifdef MB_MASTER_RTU_ENABLED

//#endif
}
