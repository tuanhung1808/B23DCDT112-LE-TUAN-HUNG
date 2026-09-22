#include "IP_TIMER.h"


 void IP_GPIO_AF_OpenDrain_50Mhz(GPIO_TypeDef *GPIOx, int pin)
{
    // 1. B?t xung nh?p (Clock) cho GPIO Port tuong ?ng
    if (GPIOx == GPIOA)      RCC->APB2ENR |= (1 << 2);
    else if (GPIOx == GPIOB) RCC->APB2ENR |= (1 << 3);
    else if (GPIOx == GPIOC) RCC->APB2ENR |= (1 << 4);
    else if (GPIOx == GPIOD) RCC->APB2ENR |= (1 << 5);

    // 2. C?u hình thanh ghi CRL (cho chân 0 - 7) ho?c CRH (cho chân 8 - 15)
    if (pin < 8)
    {
        GPIOx->CRL &= ~(0xF << (pin * 4));       // Xóa 4 bit cu c?a chân hi?n t?i
        GPIOx->CRL |=  (0xB << (pin * 4));       // Gán MODE = 11, CNF = 10 (AF Push-Pull 50MHz)
    }
    else
    {
        int shift = (pin - 8) * 4;
        GPIOx->CRH &= ~(0xF << shift);           // Xóa 4 bit cu c?a chân hi?n t?i
        GPIOx->CRH |=  (0xB << shift);           // Gán MODE = 11, CNF = 10 (AF Push-Pull 50MHz)
    }
}


static int TIMER_GetConfig(GPIO_TypeDef *GPIOx, int pin, TIM_TypeDef **timer, int *channel)
{
    if (GPIOx == GPIOA && pin >= 8 && pin <= 11) {
        RCC->APB2ENR |= (1 << 11); // B?t Clock TIM1
        *timer = TIM1;
        *channel = pin - 7;
    }
    else if (GPIOx == GPIOA && pin >= 0 && pin <= 3) {
        RCC->APB1ENR |= (1 << 0); // B?t Clock TIM2
        *timer = TIM2;
        *channel = pin + 1;
    }
    else if ((GPIOx == GPIOA && (pin == 6 || pin == 7)) || (GPIOx == GPIOB && (pin == 0 || pin == 1))) {
        RCC->APB1ENR |= (1 << 1); // B?t Clock TIM3
        *timer = TIM3;
        *channel = (GPIOx == GPIOA) ? (pin - 5) : (pin + 3);
    }
    else if (GPIOx == GPIOB && pin >= 6 && pin <= 9) {
        RCC->APB1ENR |= (1 << 2); // B?t Clock TIM4
        *timer = TIM4;
        *channel = pin - 5;
    }
    else {
        return -1; // Không h? tr? chân này
    }
    return 0;
}


static void TIMER_ConfigPWMChannel(TIM_TypeDef *tim, int channel)
{
    switch (channel) {
        case 1: tim->CCMR1 |= (6 << 4);   break; // PWM mode 1 cho Channel 1
        case 2: tim->CCMR1 |= (6 << 12);  break; // PWM mode 1 cho Channel 2
        case 3: tim->CCMR2 |= (6 << 4);   break; // PWM mode 1 cho Channel 3
        case 4: tim->CCMR2 |= (6 << 12);  break; // PWM mode 1 cho Channel 4
    }
    
    // M? Capture/Compare cho kênh tuong ?ng
    tim->CCER |= (1 << ((channel - 1) * 4)); 

    // Riêng TIM1 b?t bu?c ph?i b?t ngõ ra Main Output Enable (MOE)
    if (tim == TIM1) {
        tim->BDTR |= (1 << 15);
    }
}


static void TIMER_SetDutyCycle(TIM_TypeDef *tim, int channel, uint32_t pulse_value)
{
    switch (channel) {
        case 1: tim->CCR1 = pulse_value; break;
        case 2: tim->CCR2 = pulse_value; break;
        case 3: tim->CCR3 = pulse_value; break;
        case 4: tim->CCR4 = pulse_value; break;
    }
}

/**
 * @brief  Kh?i t?o PWM di?u khi?n LED (T?n s? ~1kHz)
 */
void TIMER_PWM_Led(GPIO_TypeDef *GPIOx, int pin, int speed)
{
    if (speed < 0)   speed = 0;
    if (speed > 100) speed = 100;

    TIM_TypeDef *tim;
    int channel;

    if (TIMER_GetConfig(GPIOx, pin, &tim, &channel) != 0) return;

    IP_GPIO_AF_OpenDrain_50Mhz(GPIOx, pin);

    // C?u hình t?n s? PWM ~1kHz (72MHz / 72 / 1000 = 1000Hz)
    tim->PSC = 72 - 1;
    tim->ARR = 1000 - 1;

    TIMER_ConfigPWMChannel(tim, channel);

    // Tính toán giá tr? xung d?a trên ARR
    uint32_t ccr_val = (speed * (tim->ARR + 1)) / 100;
    TIMER_SetDutyCycle(tim, channel, ccr_val);

    tim->EGR |= 1; // Load l?i b? chia xung
    tim->CR1 |= 1; // Kích ho?t Timer (CEN)
}


void TIMER_PWM_Motor(GPIO_TypeDef *GPIOx, int pin, int duty)
{
    if (duty < 0)   duty = 0;
    if (duty > 100) duty = 100;

    TIM_TypeDef *tim;
    int channel;

    if (TIMER_GetConfig(GPIOx, pin, &tim, &channel) != 0) return;

   IP_GPIO_AF_OpenDrain_50Mhz(GPIOx, pin);

    tim->PSC = 72 - 1;
    tim->ARR = 1000 - 1;

    TIMER_ConfigPWMChannel(tim, channel);

    uint32_t ccr_val = (duty * (tim->ARR + 1)) / 100;
    TIMER_SetDutyCycle(tim, channel, ccr_val);

    tim->EGR |= 1;
    tim->CR1 |= 1;
}


void TIMER_PWM_Servo(GPIO_TypeDef *GPIOx, int pin, int angle)
{
    if (angle < 0)   angle = 0;
    if (angle > 180) angle = 180;

    TIM_TypeDef *tim;
    int channel;

    if (TIMER_GetConfig(GPIOx, pin, &tim, &channel) != 0) return;

    IP_GPIO_AF_OpenDrain_50Mhz(GPIOx, pin);

   
    tim->PSC = 7200 - 1;
    tim->ARR = 200 - 1;

    TIMER_ConfigPWMChannel(tim, channel);

 
    uint32_t ccr_val = 1000 + (angle * 1000) / 180; 

    ccr_val = 10 + (angle * 10) / 180;

    TIMER_SetDutyCycle(tim, channel, ccr_val);

    tim->EGR |= 1;
    tim->CR1 |= 1;
}




