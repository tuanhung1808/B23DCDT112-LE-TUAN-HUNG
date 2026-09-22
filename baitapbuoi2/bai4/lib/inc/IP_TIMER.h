#ifndef __IP_TIMER_H
#define __IP_TIMER_H

#include "stm32f10x.h"


void TIMER_PWM_Led(GPIO_TypeDef *GPIOx, int pin, int speed);
void TIMER_PWM_Motor(GPIO_TypeDef *GPIOx, int pin, int duty);
void TIMER_PWM_Servo(GPIO_TypeDef *GPIOx, int pin, int angle);
void IP_GPIO_AF_OpenDrain_50Mhz(GPIO_TypeDef *GPIOx, int pin);


#endif 