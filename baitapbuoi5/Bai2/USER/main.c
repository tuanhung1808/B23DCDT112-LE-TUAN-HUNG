#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

/* =======================================================
 * CAU HINH GHI DE PRINTF RA UART1 (Dung cho Hercules)
 * ======================================================= */
struct __FILE { int handle; };
FILE __stdout;

int fputc(int ch, FILE *f) 
{
    // Doi den khi thanh ghi truyen (TX) trong
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    // Gui byte du lieu
    USART_SendData(USART1, (uint8_t) ch);
    return ch;
}

/* =======================================================
 * KHAI BAO BIEN TOAN CUC VA QUEUE
 * ======================================================= */
xQueueHandle xQueueCount;
volatile uint32_t press_count = 0;

/* =======================================================
 * HAM KHOI TAO PHAN CUNG (SPL)
 * ======================================================= */
void Hardware_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* Cau hinh nhom uu tien ngat cho FreeRTOS (Bat buoc) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 1. Cap xung nhip cho GPIOA, AFIO (EXTI) va USART1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);

    /* 2. Cau hinh UART1 (PA9: TX, PA10: RX) - Toc do 115200 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    /* 3. Cau hinh Nut nhan PA0 lam Ngat ngoai EXTI0 (Muc 1 khi nhan) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // Input Pull-Down (Mac dinh 0V)
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Ket noi chan PA0 voi bo ngat Line0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // Bat su kien canh len (0 -> 1)
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Cau hinh muc do uu tien ngat cho EXTI0
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 11; // Phai an toan voi FreeRTOS
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* =======================================================
 * HAM XU LY NGAT NGOAI (ISR) TU PA0
 * ======================================================= */
void EXTI0_IRQHandler(void) 
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    // Kiem tra dung la ngat tu Line 0
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
    {
        press_count++; // Tang bien dem so lan nhan
        
        // Gui gia tri vao Queue tu ISR
        xQueueSendFromISR(xQueueCount, (void *)&press_count, &xHigherPriorityTaskWoken);
        
        // Xoa co ngat
        EXTI_ClearITPendingBit(EXTI_Line0);

        // Chuyen ngu canh sang Task UART
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* =======================================================
 * TASK TRUYEN DU LIEU QUA UART
 * ======================================================= */
void UART_Task(void *pvParameters) 
{
    uint32_t received_count;

    while (1) 
    {
        /* Dua Task ve trang thai Blocked de cho du lieu tu Queue */
        if (xQueueReceive(xQueueCount, &received_count, portMAX_DELAY) == pdPASS) 
        {
            // Dinh dang chuoi va in ra UART
            printf("nhom14:%lu\r\n", received_count);
        }
    }
}

/* =======================================================
 * CHUONG TRINH CHINH (MAIN)
 * ======================================================= */
int main(void) 
{
    Hardware_Init();

    // Tao Queue luu tru toi da 10 phan tu kieu uint32_t
    xQueueCount = xQueueCreate(10, sizeof(uint32_t));

    // Tao Task xu ly in UART
    xTaskCreate(UART_Task, "UART_Task", 128, NULL, 1, NULL);

    // Kich hoat bo lap lich
    vTaskStartScheduler();

    while (1) {}
}