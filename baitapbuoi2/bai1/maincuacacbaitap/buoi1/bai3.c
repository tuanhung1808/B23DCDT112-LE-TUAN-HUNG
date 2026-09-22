#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"


int main(void)
{

    SystemInit();
    SysTick_Init();

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
   
    GPIOA->CRL = 0x88888888;
   
    
    GPIOB->CRH = 0x33333333;
    GPIOB->ODR = 0x00000000;
    
    
    uint8_t prev_input = 0;   
    uint8_t led_states = 0;   

    while (1)
    {
      
        uint8_t current_input = (uint8_t)(GPIOA->IDR & 0x00FF);

  
        uint8_t press_edge = current_input & ~prev_input;

        if (press_edge > 0)
        {
  
            led_states ^= press_edge;

      
            GPIOB->ODR = (uint16_t)led_states << 8;
        }

        prev_input = current_input;

   
        delay_ms(20);
    }
}
