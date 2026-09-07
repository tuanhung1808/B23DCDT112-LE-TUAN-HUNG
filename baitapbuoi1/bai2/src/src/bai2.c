#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"


int main(void)
{
SystemInit();
SysTick_Init();

    
   RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
   GPIOA->CRL &= ~0xFFFFFFFF;
   GPIOA->CRL |= 0x33333333;
   GPIOA->ODR =  0x0;

	while(1){
     for(int i=0; i<8; i++){
        GPIOA->ODR |= (1<<i);
        delay_ms(250);  
     }
     GPIOA->ODR =  0x0;
     delay_ms(100);
     GPIOA->ODR =  0xFF;
     delay_ms(100);
     GPIOA->ODR =  0x0;
     delay_ms(100);

     for(int i=7; i>=0; i--){
        GPIOA->ODR |= (1<<i);
        delay_ms(250);  
     }

     GPIOA->ODR =  0x0;
     delay_ms(100);
     GPIOA->ODR =  0xFF;
     delay_ms(100);
     GPIOA->ODR =  0x0;
     delay_ms(100);

    }



}
