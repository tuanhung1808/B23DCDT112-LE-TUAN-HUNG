#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"


int main(void)
{

    SystemInit();
    SysTick_Init();

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
   
    GPIOA->CRL = 0x08;
   
    
    GPIOB->CRH = 0x03;
    GPIOB->ODR = 0x0;
    
    
    uint8_t prev_input = 0;   

    while (1)
    {
      
        uint8_t current_input = (GPIOA->IDR & 0x01);



        if (current_input == 1 && prev_input == 0)
        {
    
      
            GPIOB->ODR ^= (1<<8);
        }

        prev_input = current_input;

   
        delay_ms(20);
    }
}
