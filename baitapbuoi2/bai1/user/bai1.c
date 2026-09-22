#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>



char rx_buffer[100];               
volatile uint8_t rx_index = 0;     
volatile uint8_t data_ready = 0;   


void USART1_IRQHandler(void)
{
    

    if (USART1->SR & (1 << 5))
    {
        char c = (char)(USART1->DR & 0xFF); 
        
        GPIOC->ODR ^= (1<<13);

        
        if (c == '!') 
        {
            rx_buffer[rx_index] = '\0'; 
            data_ready = 1;             
            rx_index = 0;               
        }
        else
        {
            if (rx_index < 99) 
            {
                rx_buffer[rx_index++] = c;
            }
        }
    }
}



int main(void)
{



    SystemInit();
    SysTick_Init();

 Config_Uart(1); 

     RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
   GPIOC->CRH &= ~(0xF << 20);
   GPIOC->CRH |= (0x3 << 20);



 USART1->CR1 |= (1 << 5); 
    NVIC_EnableIRQ(USART1_IRQn);
    __asm volatile ("cpsie i");

    while (1)
    {
        if (data_ready == 1)
        {
            
            USART_SendString(USART1, "<INT1341><14>: ");
            
            
            USART_SendString(USART1, rx_buffer);
            
            
            USART_SendString(USART1, "\n\r"); 

            data_ready = 0; 
            rx_buffer[0] = '\0';
        }


    }





}
