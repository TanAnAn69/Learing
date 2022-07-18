/*************************************************************************
 * File     : bsp_commrtos.c
 * Author   : platform team
 * COPYRIGHT (C) 2019, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2019-01-21      aaron           first version
 ************************************************************************/

#include <rtthread.h>	//系统自带类型及函数
#include <rthw.h>		//开关中断及文本切换
#include "stm32f4xx.h"
#include "bsp_commrtos.h"

#define COMM_COUNT_MAX 6
rt_uint8_t TxFlag1 = RT_FALSE;
rt_uint8_t TxFlag2 = RT_FALSE;
rt_uint8_t TxFlag3 = RT_FALSE;
rt_uint8_t TxFlag4 = RT_FALSE;
rt_uint8_t TxFlag5 = RT_FALSE;
rt_uint8_t TxFlag6 = RT_FALSE;

typedef struct
{
	rt_uint32_t RingBufRxCtr;				/* Number of characters in the Rx ring buffer   		   */
	struct rt_semaphore RingBufRxSem; 			   /* Pointer to Rx semaphore   							  */
	rt_uint8_t *RingBufRxInPtr;  			/* Pointer to where next character will be inserted 	   */
	rt_uint8_t *RingBufRxOutPtr; 			/* Pointer from where next character will be extracted     */
	rt_uint8_t RingBufRx[COMM_RX_BUF_SIZE]; /* Ring buffer character storage (Rx)					  */

	rt_uint32_t RingBufTxCtr;				/* Number of characters in the Tx ring buffer   		   */
	struct rt_semaphore RingBufTxSem; 			   /* Pointer to Tx semaphore   							  */
	rt_uint8_t *RingBufTxInPtr;  			/* Pointer to where next character will be inserted 	   */
	rt_uint8_t *RingBufTxOutPtr; 			/* Pointer from where next character will be extracted     */
	rt_uint8_t RingBufTx[COMM_TX_BUF_SIZE]; /* Ring buffer character storage (Tx)					  */
} COMM_RING_BUF;
//是否定义按字节对齐
//基本上使用静态的定义，因为所有功能都会使用到，同时也可以查看静态的RAM使用情况

static COMM_RING_BUF CommRingBuf[COMM_COUNT_MAX];	//总共6个串口

rt_uint8_t TxFlag[COMM_COUNT_MAX];

rt_uint8_t comm_get_char(rt_uint8_t ch, rt_int32_t iTime, rt_uint8_t *err)
{
	rt_base_t level;
	rt_uint8_t c;
	rt_err_t oserr;
	COMM_RING_BUF *pbuf;

	if (ch >= COMM_COUNT_MAX) {
		*err = COMM_BAD_CH;		  //无效的通信接口
		return (NUL);
	}
	pbuf = &(CommRingBuf[ch]);	  //pbuf指向CommRingBuf[ch]结构体的首址 static COMM_RING_BUF CommRingBuf[COMM_COUNT_MAX]
    
	oserr = rt_sem_take(&pbuf->RingBufRxSem, iTime);
	if (oserr == -RT_ETIMEOUT) 
    {
		/* See if characters received within timeout*/
		*err = COMM_RX_TIMEOUT; 					   /* No, return error code 				   */
		return (NUL);
	} 
    else 
    {
	  	level = rt_hw_interrupt_disable();
	  	pbuf->RingBufRxCtr--;   						   /* Yes, decrement character count		   */
	  	c = *pbuf->RingBufRxOutPtr++;   				   /* Get character from buffer 			   */
		if (pbuf->RingBufRxOutPtr == &pbuf->RingBufRx[COMM_RX_BUF_SIZE])
        {
			/* Wrap OUT pointer 	*/
			pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
		}
	  	rt_hw_interrupt_enable(level);
	  	*err = COMM_NO_ERR;
	  	return (c);
	}
}


rt_uint8_t comm_get_tx_char(rt_uint8_t ch, rt_uint8_t *err)
{
	rt_uint8_t c;
	COMM_RING_BUF *pbuf;

	if (ch >= COMM_COUNT_MAX) {
		*err = COMM_BAD_CH;
		return (NUL);
	}
	pbuf = &(CommRingBuf[ch]);

	if (pbuf->RingBufTxCtr > 0) {
		/* See if buffer is empty   				*/
		pbuf->RingBufTxCtr--;   						   /* No, decrement character count 		   */
		c = *pbuf->RingBufTxOutPtr++;   				   /* Get character from buffer 			   */
		if (pbuf->RingBufTxOutPtr == &pbuf->RingBufTx[COMM_TX_BUF_SIZE]) {
			/* Wrap OUT pointer 	*/
			pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
		}
		rt_sem_release(&pbuf->RingBufTxSem);
		*err = COMM_NO_ERR;
		return (c); 									   /* Characters are still available		   */
	} else {
	  	*err = COMM_TX_EMPTY;
	  	return (NUL);   								   /* Buffer is empty   					   */
	}
}

void comm_init(uint8_t iUsart)
{
	COMM_RING_BUF *pbuf;

    switch (iUsart)
    {
    case UART1:
        pbuf = &CommRingBuf[0];   				  
        pbuf->RingBufRxCtr = 0;
        pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        rt_sem_init(&pbuf->RingBufRxSem, "Comm0RxSem", 0, RT_IPC_FLAG_FIFO);
        pbuf->RingBufTxCtr = 0;
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
        pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        rt_sem_init(&pbuf->RingBufTxSem, "Comm0TxSem", COMM_TX_BUF_SIZE, RT_IPC_FLAG_FIFO);
    break;
    case UART2:
        pbuf = &CommRingBuf[1];   				  
        pbuf->RingBufRxCtr = 0;
        pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        rt_sem_init(&pbuf->RingBufRxSem, "Comm1RxSem", 0, RT_IPC_FLAG_FIFO);
        pbuf->RingBufTxCtr = 0;
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
        pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        rt_sem_init(&pbuf->RingBufTxSem, "Comm1TxSem", COMM_TX_BUF_SIZE, RT_IPC_FLAG_FIFO);
    break;
    case UART3:
        pbuf = &CommRingBuf[2];   				  
        pbuf->RingBufRxCtr = 0;
        pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        rt_sem_init(&pbuf->RingBufRxSem, "Comm2RxSem", 0, RT_IPC_FLAG_FIFO);
        pbuf->RingBufTxCtr = 0;
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
        pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        rt_sem_init(&pbuf->RingBufTxSem, "Comm2TxSem", COMM_TX_BUF_SIZE, RT_IPC_FLAG_FIFO);
    break;
    case USART4:
        pbuf = &CommRingBuf[3];   				  
        pbuf->RingBufRxCtr = 0;
        pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        rt_sem_init(&pbuf->RingBufRxSem, "Comm3RxSem", 0, RT_IPC_FLAG_FIFO);
        pbuf->RingBufTxCtr = 0;
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
        pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        rt_sem_init(&pbuf->RingBufTxSem, "Comm3TxSem", COMM_TX_BUF_SIZE, RT_IPC_FLAG_FIFO);
    break;
    case USART5:
        pbuf = &CommRingBuf[4];   				  
        pbuf->RingBufRxCtr = 0;
        pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        rt_sem_init(&pbuf->RingBufRxSem, "Comm4RxSem", 0, RT_IPC_FLAG_FIFO);
        pbuf->RingBufTxCtr = 0;
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
        pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        rt_sem_init(&pbuf->RingBufTxSem, "Comm4TxSem", COMM_TX_BUF_SIZE, RT_IPC_FLAG_FIFO);
    break;
    case UART6:
        pbuf = &CommRingBuf[5];   				  
        pbuf->RingBufRxCtr = 0;
        pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
        pbuf->RingBufRxOutPtr = &pbuf->RingBufRx[0];
        rt_sem_init(&pbuf->RingBufRxSem, "Comm5RxSem", 0, RT_IPC_FLAG_FIFO);
        pbuf->RingBufTxCtr = 0;
        pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
        pbuf->RingBufTxOutPtr = &pbuf->RingBufTx[0];
        rt_sem_init(&pbuf->RingBufTxSem, "Comm5TxSem", COMM_TX_BUF_SIZE, RT_IPC_FLAG_FIFO);
    break;
    
    default:
        break;
    }
}


rt_bool_t comm_is_empty(rt_uint8_t ch)
{
	rt_base_t level;
	rt_bool_t empty;
	COMM_RING_BUF *pbuf;

	if (ch >= COMM_COUNT_MAX) {
		return (0xff);
	}
	pbuf = &(CommRingBuf[ch]);

	level = rt_hw_interrupt_disable();
	if (pbuf->RingBufRxCtr > 0) {
		/* See if buffer is empty   				*/
		empty = RT_FALSE;								   /* Buffer is NOT empty   				   */
	} else {
	  	empty = RT_TRUE; 								   /* Buffer is empty   					   */
	}
	rt_hw_interrupt_enable(level);
	return (empty);
}


rt_bool_t comm_is_full(rt_uint8_t ch)
{
	rt_base_t level;
	rt_bool_t full;
	COMM_RING_BUF *pbuf;

	if (ch >= COMM_COUNT_MAX) {
		return (0xff);
	}
	pbuf = &(CommRingBuf[ch]);


	level = rt_hw_interrupt_disable();
	if (pbuf->RingBufTxCtr < COMM_TX_BUF_SIZE) {
		/* See if buffer is full					*/
		full = RT_FALSE;								   /* Buffer is NOT full					   */
	} else {
	  	full = RT_TRUE;								   /* Buffer is full						   */
	}
	rt_hw_interrupt_enable(level);
	return (full);
}

//rt_uint8_t comm_put_char(rt_uint8_t ch, rt_uint8_t c, rt_int32_t to)
//{
//	rt_base_t level;
//	rt_err_t oserr;
//	COMM_RING_BUF *pbuf;

//	if (ch >= COMM_COUNT_MAX) 
//	{
//		return (COMM_BAD_CH);
//	}
//	pbuf = &(CommRingBuf[ch]);

//	oserr = rt_sem_take(&pbuf->RingBufTxSem, to);
//	if (oserr == -RT_ETIMEOUT) 
//	{
//		return (COMM_TX_TIMEOUT);   					   /* Timed out, return error code  		   */
//	}
//	level = rt_hw_interrupt_disable();
//	pbuf->RingBufTxCtr++;   							   /* No, increment character count 		   */
//	*pbuf->RingBufTxInPtr++ = c;						   /* Put character into buffer 			   */
//	if (pbuf->RingBufTxInPtr == &pbuf->RingBufTx[COMM_TX_BUF_SIZE]) 
//	{
//		/* Wrap IN pointer*/
//		pbuf->RingBufTxInPtr = &pbuf->RingBufTx[0];
//	}
//	rt_hw_interrupt_enable(level);
//	switch (ch)
//	{
//		case UART1 :
//			if(TxFlag1 == RT_FALSE)
//			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
//			break ;	
//		case UART2 :
//			if(TxFlag2 == RT_FALSE)
//			USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
//			break ;	
//		case UART3 :
//			if(TxFlag3 == RT_FALSE)
//			USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
//			break ;	
//		case USART4 :
//			if(TxFlag4 == RT_FALSE)  
//			USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
//			break ;	
//		case USART5 :
//			if(TxFlag5 == RT_FALSE)
//			USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
//			break ;	
//		default :
//				break;
//	}
//	return (COMM_NO_ERR);
//}


void comm_put_rx_char(rt_uint8_t ch, rt_uint8_t c)
{
	COMM_RING_BUF *pbuf;
	rt_base_t level;

	if (ch >= COMM_COUNT_MAX) {
		return;
	}
	pbuf = &(CommRingBuf[ch]);
	level = rt_hw_interrupt_disable();
	if (pbuf->RingBufRxCtr < COMM_RX_BUF_SIZE)
    {
		/* See if buffer is full					*/
		pbuf->RingBufRxCtr++;   						   /* No, increment character count 		   */
		*pbuf->RingBufRxInPtr++ = c;					   /* Put character into buffer 			   */
		if (pbuf->RingBufRxInPtr == &pbuf->RingBufRx[COMM_RX_BUF_SIZE])
        {
			/* Wrap IN pointer  		 */
			pbuf->RingBufRxInPtr = &pbuf->RingBufRx[0];
		}
		rt_sem_release(&pbuf->RingBufRxSem);
	}
	rt_hw_interrupt_enable(level);
}

void clear_comm_rx(rt_uint8_t ch)
{
	rt_uint8_t err;
	if (ch >= COMM_COUNT_MAX) {
		return;
	}
	while (comm_is_empty(ch) == RT_FALSE) {
		comm_get_char(ch, RT_WAITING_FOREVER, &err);
	}
}




