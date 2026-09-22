#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>





int main(void)
{
SystemInit();
    SysTick_Init();

 Config_Uart(1); 



ADC_Config(ADC1,GPIOA,4);
	char str[10];

int val=0;
	
	while(1){
   val=ADC_Rx(ADC1,GPIOA,4);
		int2char(val, str);
		

		USART_SendString(USART1, str);
		USART_SendString(USART1, "\n");

			delay_ms(1000);
		
	}

	
}

