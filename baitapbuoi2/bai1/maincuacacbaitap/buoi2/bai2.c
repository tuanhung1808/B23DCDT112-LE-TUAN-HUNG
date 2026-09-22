#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
// #include "SYSTICK.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>



volatile uint32_t counter_10Hz = 0;
volatile uint32_t counter_1Hz = 0;
volatile uint32_t counter_0_1Hz = 0;


void GPIO_LED_Init(void)
{
    
    RCC->APB2ENR |= (1 << 2);


    GPIOA->CRL &= ~(0xFFF); 
    

    GPIOA->CRL |= 0x222; 
}


void SysTick_Init(void)
{

    SysTick->LOAD = 71999; 
 
    SysTick->VAL = 0;      
    
    SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}


void SysTick_Handler(void)
{
    counter_10Hz++;
    counter_1Hz++;
    counter_0_1Hz++;

  
    if (counter_10Hz >= 50)
    {
        GPIOA->ODR ^= (1 << 0); 
        counter_10Hz = 0;       
    }

    if (counter_1Hz >= 500)
    {
        GPIOA->ODR ^= (1 << 1); 
        counter_1Hz = 0;        
    }

    if (counter_0_1Hz >= 5000)
    {
        GPIOA->ODR ^= (1 << 2); 
        counter_0_1Hz = 0;       
    }
}

int main(void)
{
    SystemInit();
    GPIO_LED_Init();
    SysTick_Init();

    while (1)
    {
        // Các công việc khác xử lý ở đây
    }
}

