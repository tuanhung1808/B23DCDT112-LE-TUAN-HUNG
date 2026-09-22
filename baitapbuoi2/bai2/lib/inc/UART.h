
#ifndef __UART_LIB_H_
#define __UART_LIB_H_


#include "stm32f10x.h"             

void Config_Uart(uint8_t uart);
void USART_SendChar(USART_TypeDef *USARTx, char c);
void USART_SendString(USART_TypeDef *USARTx, char *str); 
char USART_ReceiveChar(USART_TypeDef *USARTx);




#endif

