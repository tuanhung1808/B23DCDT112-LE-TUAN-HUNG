
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"


int main(void)
{
SystemInit();
SysTick_Init();

    
   RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
   GPIOC->CRH &= ~(0xF << 20);
   GPIOC->CRH |= (0x3 << 20);
   GPIOC->ODR =  0x0;

	while(1){
        GPIOC->ODR ^= (1<<13);
        delay_ms(1000);  

    }



}

