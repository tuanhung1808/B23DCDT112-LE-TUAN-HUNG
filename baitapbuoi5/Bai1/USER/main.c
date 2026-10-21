#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Khai báo 3 Queue d? truy?n d? li?u d? sáng (0-100%) cho 3 LED */
xQueueHandle xQueueLED1;
xQueueHandle xQueueLED2;
xQueueHandle xQueueLED3;

/* Hàm c?u hình chân GPIO và Timer PWM */
void Hardware_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* 1. B?t xung nh?p */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* 2. C?u hình 3 chân LED (PA0, PA1, PA2) - Alternate Function Push-Pull cho PWM */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. C?u hình 3 chân Nút nh?n (PA5, PA6, PA7) - Input Pull-Down (vì nh?n vào m?c 1) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 4. C?u hình Timer 2 bam xung PWM t?n s? 1kHz */
    // Gi? s? xung nh?p h? th?ng là 72MHz. T?n s? = 72M / (71+1) / (999+1) = 1000Hz (1kHz)
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* C?u hình 3 Kênh PWM Mode 1 cho PA0(CH1), PA1(CH2), PA2(CH3) */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 0; // Ð? sáng ban d?u = 0

    TIM_OC1Init(TIM2, &TIM_OCInitStructure); // CH1 -> PA0
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM2, &TIM_OCInitStructure); // CH2 -> PA1
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM2, &TIM_OCInitStructure); // CH3 -> PA2
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_Cmd(TIM2, ENABLE);
}

/* =======================================================
 * TASK NÚT NH?N (Phát hi?n phím và g?i d? li?u vào Queue)
 * ======================================================= */
void Button_Task(void *pvParameters)
{
    uint8_t selected_led = 0; // 0: LED1, 1: LED2, 2: LED3
    uint8_t brightness[3] = {0, 0, 0}; // M?ng luu d? sáng hi?n t?i c?a 3 LED (0-100)
    
    // Tr?ng thái nút nh?n chu k? tru?c d? phát hi?n su?n lên (0 -> 1)
    uint8_t btn_sel_last = 0, btn_up_last = 0, btn_down_last = 0;

    while (1)
    {
        uint8_t btn_sel = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5); // Nút chuy?n LED
        uint8_t btn_up  = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6); // Nút tang
        uint8_t btn_down = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7); // Nút gi?m

        // 1. X? lý nút Chuy?n LED (PA5)
        if (btn_sel == 1 && btn_sel_last == 0) {
            selected_led++;
            if (selected_led > 2) selected_led = 0;
        }

        // 2. X? lý nút Tang d? sáng (PA6)
        if (btn_up == 1 && btn_up_last == 0) {
            if (brightness[selected_led] < 100) {
                brightness[selected_led] += 10; // Tang m?i bu?c 10%
            }
            // G?i d? li?u vào Queue tuong ?ng
            if (selected_led == 0) xQueueSend(xQueueLED1, &brightness[0], 0);
            if (selected_led == 1) xQueueSend(xQueueLED2, &brightness[1], 0);
            if (selected_led == 2) xQueueSend(xQueueLED3, &brightness[2], 0);
        }

        // 3. X? lý nút Gi?m d? sáng (PA7)
        if (btn_down == 1 && btn_down_last == 0) {
            if (brightness[selected_led] > 0) {
                brightness[selected_led] -= 10;
            }
            if (selected_led == 0) xQueueSend(xQueueLED1, &brightness[0], 0);
            if (selected_led == 1) xQueueSend(xQueueLED2, &brightness[1], 0);
            if (selected_led == 2) xQueueSend(xQueueLED3, &brightness[2], 0);
        }

        // Luu tr?ng thái cho l?n quét sau
        btn_sel_last = btn_sel;
        btn_up_last = btn_up;
        btn_down_last = btn_down;

        vTaskDelay(20); // Delay 20ms d? ch?ng d?i phím (Debounce)
    }
}

/* =======================================================
 * CÁC TASK ÐI?U KHI?N LED (Nh?n d? li?u t? Queue và set PWM)
 * ======================================================= */
void LED1_Task(void *pvParameters)
{
    uint8_t receive_brightness;
    while (1) {
        // Ch? vô h?n (portMAX_DELAY) cho d?n khi có d? li?u trong Queue
        if (xQueueReceive(xQueueLED1, &receive_brightness, portMAX_DELAY)) {
            TIM2->CCR1 = receive_brightness * 10; // Ð?i t? 0-100% sang 0-1000 c?a thanh ghi
        }
    }
}

void LED2_Task(void *pvParameters)
{
    uint8_t receive_brightness;
    while (1) {
        if (xQueueReceive(xQueueLED2, &receive_brightness, portMAX_DELAY)) {
            TIM2->CCR2 = receive_brightness * 10;
        }
    }
}

void LED3_Task(void *pvParameters)
{
    uint8_t receive_brightness;
    while (1) {
        if (xQueueReceive(xQueueLED3, &receive_brightness, portMAX_DELAY)) {
            TIM2->CCR3 = receive_brightness * 10;
        }
    }
}

int main(void)
{
    /* Kh?i t?o ph?n c?ng */
    Hardware_Config();

    /* Kh?i t?o 3 Queue ch?a ki?u d? li?u uint8_t, d? dài m?i Queue là 1 ph?n t? */
    xQueueLED1 = xQueueCreate(1, sizeof(uint8_t));
    xQueueLED2 = xQueueCreate(1, sizeof(uint8_t));
    xQueueLED3 = xQueueCreate(1, sizeof(uint8_t));

    /* T?o Task Nút nh?n */
    xTaskCreate(Button_Task, "Button", 128, NULL, 2, NULL);

    /* T?o 3 Task LED d?c l?p theo yêu c?u */
    xTaskCreate(LED1_Task, "LED1", 128, NULL, 1, NULL);
    xTaskCreate(LED2_Task, "LED2", 128, NULL, 1, NULL);
    xTaskCreate(LED3_Task, "LED3", 128, NULL, 1, NULL);

    /* Kích ho?t h? di?u hành ch?y */
    vTaskStartScheduler();

    while (1) {}
}