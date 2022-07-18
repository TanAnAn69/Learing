#ifndef __COMMRTOS_H__
#define __COMMRTOS_H__

#include <stdint.h>
#include <rtdef.h>
#define  COMM_RX_BUF_SIZE      256  			  /* Number of characters in Rx ring buffer 			*/
#define  COMM_TX_BUF_SIZE      256  			  /* Number of characters in Tx ring buffer 			*/

#ifndef  NUL
#define  NUL				 0x00
#endif

#define UART1 0
#define UART2 1
#define UART3 2
#define USART4 3 
#define USART5 4
#define UART6 5


extern rt_uint8_t TxFlag1 ;
extern rt_uint8_t TxFlag2 ;
extern rt_uint8_t TxFlag3 ;
extern rt_uint8_t TxFlag4 ;
extern rt_uint8_t TxFlag5 ;
extern rt_uint8_t TxFlag6 ;

/* ERROR CODES  									  */
#define  COMM_NO_ERR			0   			 /* Function call was successful					   */
#define  COMM_BAD_CH			1   			 /* Invalid communications port channel 			   */
#define  COMM_RX_EMPTY  		2   			 /* Rx buffer is empty, no character available  	   */
#define  COMM_TX_FULL   		3   			 /* Tx buffer is full, could not deposit character     */
#define  COMM_TX_EMPTY  		4   			 /* If the Tx buffer is empty.  					   */
#define  COMM_RX_TIMEOUT		5   			 /* If a timeout occurred while waiting for a character*/
#define  COMM_TX_TIMEOUT		6   			 /* If a timeout occurred while waiting to send a char.*/


#define  COMM_PARITY_NONE   	0   			 /* Defines for setting parity  					   */
#define  COMM_PARITY_ODD		1
#define  COMM_PARITY_EVEN   	2


extern rt_uint8_t comm_get_char(rt_uint8_t ch, rt_int32_t to, rt_uint8_t *err);
extern rt_uint8_t comm_get_tx_char(rt_uint8_t ch, rt_uint8_t *err);
extern void comm_init(uint8_t iUsart);
extern rt_bool_t comm_is_empty(rt_uint8_t ch);
extern rt_bool_t comm_is_full(rt_uint8_t ch);
extern rt_uint8_t comm_put_char(rt_uint8_t ch, rt_uint8_t c, rt_int32_t to);
extern void comm_put_rx_char(rt_uint8_t ch, rt_uint8_t c);
extern void clear_comm_rx(rt_uint8_t ch);

#endif
