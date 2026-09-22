#include "stm32f10x.h"
#include "UART.h"

void Config_Uart(uint8_t uart)
{
    USART_TypeDef *USARTx;

    /* Enable AFIO */
    RCC->APB2ENR |= (1 << 0); //[cite: 1]

    if (uart == 1)
    {
        /* Enable clock USART1 + GPIOA */
        RCC->APB2ENR |= (1 << 14) | (1 << 2); //[cite: 1]
        USARTx = USART1;
        USARTx->BRR = 0x1D4C; // 72MHz / (16 * 9600) = 468.75 -> 0x1D4C[cite: 1]

        /* TX: PA9 (Alternate Function Push-Pull 50MHz) */
        GPIOA->CRH &= ~(0xF << 4); //[cite: 1]
        GPIOA->CRH |=  (0xB << 4); //[cite: 1]

        /* RX: PA10 (Input floating) */
        GPIOA->CRH &= ~(0xF << 8); //[cite: 1]
        GPIOA->CRH |=  (0x4 << 8); //[cite: 1]
    }
    else if (uart == 2)
    {
        RCC->APB1ENR |= (1 << 17); // USART2[cite: 1]
        RCC->APB2ENR |= (1 << 2);  // GPIOA[cite: 1]
        USARTx = USART2;
        USARTx->BRR = 0x0EA6; // 36MHz / (16 * 9600) = 234.375 -> 0x0EA6[cite: 1]

        /* TX: PA2 */
        GPIOA->CRL &= ~(0xF << 8); //[cite: 1]
        GPIOA->CRL |=  (0xB << 8); //[cite: 1]

        /* RX: PA3 */
        GPIOA->CRL &= ~(0xF << 12); //[cite: 1]
        GPIOA->CRL |=  (0x4 << 12); //[cite: 1]
    }
    else if (uart == 3)
    {
        RCC->APB1ENR |= (1 << 18); // USART3[cite: 1]
        RCC->APB2ENR |= (1 << 3);  // GPIOB[cite: 1]
        USARTx = USART3;
        USARTx->BRR = 0x0EA6; // 36MHz / (16 * 9600) = 234.375 -> 0x0EA6[cite: 1]

        /* TX: PB10 */
        GPIOB->CRH &= ~(0xF << 8); //[cite: 1]
        GPIOB->CRH |=  (0xB << 8); //[cite: 1]

        /* RX: PB11 */
        GPIOB->CRH &= ~(0xF << 12); //[cite: 1]
        GPIOB->CRH |=  (0x4 << 12); //[cite: 1]
    }
    else return; //[cite: 1]

    /* USART config */
    USARTx->CR1 |= (1 << 3) | (1 << 2) | (1 << 13); // TE, RE, UE[cite: 1]
    USARTx->CR2 = 0x0000;      // 1 stop bit[cite: 1]
    USARTx->CR3 = 0x0000;      // No flow control[cite: 1]
}


void USART_SendChar(USART_TypeDef *USARTx, char c)
{
    // 1. Ch? cho d?n khi thanh ghi d? li?u truy?n (TXE - bit 7) r?nh
    while (!(USARTx->SR & (1 << 7))); 
    
    // 2. N?p d? li?u v�o thanh ghi DR
    USARTx->DR = (c & 0xFF);
}

/* G?i chu?i k� t? */
void USART_SendString(USART_TypeDef *USARTx, char *str)
{
    while (*str)
    {
        USART_SendChar(USARTx, *str++); //[cite: 1]
    }
}

/* B? sung: H�m nh?n 1 k� t? t? UART */
char USART_ReceiveChar(USART_TypeDef *USARTx)
{
    // Ch? cho d?n khi thanh ghi nh?n c� d? li?u (RXNE - bit 5)
    while (!(USARTx->SR & (1 << 5))); 
    
    // Tr? v? d? li?u nh?n du?c trong thanh ghi DR
    return (char)(USARTx->DR & 0xFF);
}