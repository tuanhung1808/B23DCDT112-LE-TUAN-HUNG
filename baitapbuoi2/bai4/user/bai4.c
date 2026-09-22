#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSTICK.h"
#include "UART.h"
#include <stdio.h>
#include <string.h>
#include "IP_TIMER.h"



int main(void)
{

    SystemInit();


    TIMER_PWM_Motor(GPIOA, 0, 10);  // PA0 (Channel 1) - Duty 10%
    TIMER_PWM_Motor(GPIOA, 1, 30);  // PA1 (Channel 2) - Duty 30%
    TIMER_PWM_Motor(GPIOA, 2, 50);  // PA2 (Channel 3) - Duty 50%
    TIMER_PWM_Motor(GPIOA, 3, 70);  // PA3 (Channel 4) - Duty 70%

    while (1)
    {
       
    }
}



