#include "stm32f10x.h"

/*
 * ============================================================
 * STM32F103
 *
 * LED:
 *   TIM2_CH1 -> PA0
 *
 * UART:
 *   USART1_TX -> PA9
 *   USART1_RX -> PA10
 *
 * PWM:
 *   Frequency = 1 kHz
 *
 * Commands:
 *   ON!
 *   OFF!
 *   PWM:30%!
 *   PWM:70%!
 *   Status!
 *
 * ============================================================
 */


/* ============================================================
 * BIẾN TRẠNG THÁI
 * ============================================================
 *
 * led_state:
 *   0 -> OFF
 *   1 -> ON
 *
 * pwm_percent:
 *   Mức PWM đang được cấu hình.
 *
 * Ví dụ:
 *
 *   led_state  = 0
 *   pwm_percent = 50
 *
 * nghĩa là:
 *
 *   LED đang OFF
 *   nhưng mức PWM đã lưu là 50%.
 *
 * Khi ON -> LED sẽ sáng 50%.
 */

volatile uint8_t led_state = 0;
volatile uint8_t pwm_percent = 50;


/* ============================================================
 * BUFFER UART
 * ============================================================
 */

volatile char uart_buffer[32];
volatile uint8_t uart_index = 0;


/* ============================================================
 * UART SEND CHAR
 * ============================================================
 */

void USART1_SendChar(char c)
{
    while (!(USART1->SR & (1 << 7)));

    USART1->DR = c;
}


/* ============================================================
 * UART SEND STRING
 * ============================================================
 */

void USART1_SendString(char *str)
{
    while (*str)
    {
        USART1_SendChar(*str);
        str++;
    }
}


/* ============================================================
 * UART SEND NUMBER
 * ============================================================
 */

void USART1_SendNumber(uint32_t number)
{
    char buffer[10];
    uint8_t i = 0;

    if (number == 0)
    {
        USART1_SendChar('0');
        return;
    }

    while (number > 0)
    {
        buffer[i] = '0' + (number % 10);
        number /= 10;
        i++;
    }

    while (i > 0)
    {
        i--;
        USART1_SendChar(buffer[i]);
    }
}


/* ============================================================
 * PWM SET DUTY
 * ============================================================
 *
 * ARR = 999
 *
 * 10% -> CCR1 = 100
 * 30% -> CCR1 = 300
 * 50% -> CCR1 = 500
 * 70% -> CCR1 = 700
 *
 * Công thức:
 *
 * CCR1 = pwm_percent * 10
 */

void PWM_SetDuty(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }

    pwm_percent = percent;

    /*
     * Nếu LED đang ON
     * thì thay đổi CCR ngay lập tức.
     *
     * Nếu LED đang OFF:
     * vẫn lưu pwm_percent
     * nhưng không cho LED sáng.
     */
    if (led_state == 1)
    {
        TIM2->CCR1 = percent * 10;
    }
}


/* ============================================================
 * LED ON
 * ============================================================
 */

void LED_ON(void)
{
    led_state = 1;

    /*
     * Khi ON:
     * dùng mức PWM gần nhất đã lưu.
     */
    TIM2->CCR1 = pwm_percent * 10;
}


/* ============================================================
 * LED OFF
 * ============================================================
 */

void LED_OFF(void)
{
    led_state = 0;

    /*
     * Tắt LED thực tế.
     *
     * Không xóa pwm_percent.
     *
     * Ví dụ:
     *
     * pwm_percent = 70
     *
     * OFF -> CCR1 = 0
     *
     * nhưng pwm_percent vẫn = 70.
     *
     * Sau ON -> CCR1 = 700.
     */

    TIM2->CCR1 = 0;
}


/* ============================================================
 * XỬ LÝ LỆNH UART
 * ============================================================
 */

void Process_Command(char *cmd)
{
    /*
     * --------------------------------------------------------
     * LỆNH ON
     * --------------------------------------------------------
     */

    if ((cmd[0] == 'O') &&
        (cmd[1] == 'N') &&
        (cmd[2] == '\0'))
    {
        LED_ON();

        USART1_SendString("LED ON, PWM = ");
        USART1_SendNumber(pwm_percent);
        USART1_SendString("%\r\n");

        return;
    }


    /*
     * --------------------------------------------------------
     * LỆNH OFF
     * --------------------------------------------------------
     */

    if ((cmd[0] == 'O') &&
        (cmd[1] == 'F') &&
        (cmd[2] == 'F') &&
        (cmd[3] == '\0'))
    {
        LED_OFF();

        USART1_SendString("LED OFF, PWM = ");
        USART1_SendNumber(pwm_percent);
        USART1_SendString("%\r\n");

        return;
    }


    /*
     * --------------------------------------------------------
     * LỆNH STATUS
     * --------------------------------------------------------
     */

    if ((cmd[0] == 'S') &&
        (cmd[1] == 't') &&
        (cmd[2] == 'a') &&
        (cmd[3] == 't') &&
        (cmd[4] == 'u') &&
        (cmd[5] == 's') &&
        (cmd[6] == '\0'))
    {
        USART1_SendString("LED = ");

        if (led_state == 1)
        {
            USART1_SendString("ON");
        }
        else
        {
            USART1_SendString("OFF");
        }

        USART1_SendString(", PWM = ");
        USART1_SendNumber(pwm_percent);
        USART1_SendString("%\r\n");

        return;
    }


    /*
     * --------------------------------------------------------
     * LỆNH PWM:xx%
     *
     * Ví dụ:
     *
     * PWM:10%
     * PWM:30%
     * PWM:50%
     * PWM:70%
     * PWM:100%
     *
     * cmd:
     *
     * P W M : 3 0 %
     * 0 1 2 3 4 5 6
     * --------------------------------------------------------
     */

    if ((cmd[0] == 'P') &&
        (cmd[1] == 'W') &&
        (cmd[2] == 'M') &&
        (cmd[3] == ':'))
    {
        uint8_t percent = 0;
        uint8_t i = 4;

        /*
         * Đọc các chữ số.
         *
         * Ví dụ:
         *
         * "PWM:30%"
         *
         * i = 4 -> '3'
         * i = 5 -> '0'
         */

        while ((cmd[i] >= '0') &&
               (cmd[i] <= '9'))
        {
            percent = percent * 10 + (cmd[i] - '0');
            i++;
        }

        /*
         * Sau số phải là '%'
         */

        if (cmd[i] == '%')
        {
            /*
             * Kiểm tra 0..100%
             */

            if (percent <= 100)
            {
                PWM_SetDuty(percent);

                USART1_SendString("PWM = ");
                USART1_SendNumber(pwm_percent);
                USART1_SendString("%\r\n");

                return;
            }
        }
    }


    /*
     * --------------------------------------------------------
     * LỆNH KHÔNG HỢP LỆ
     * --------------------------------------------------------
     */

    USART1_SendString("ERROR\r\n");
}


/* ============================================================
 * USART1 IRQ HANDLER
 * ============================================================
 *
 * Đây là phần quan trọng nhất của yêu cầu:
 *
 * UART nhận bằng NGẮT.
 *
 * Khi PC gửi từng ký tự:
 *
 * O
 * N
 * !
 *
 * CPU sẽ nhảy vào:
 *
 * USART1_IRQHandler()
 *
 */

void USART1_IRQHandler(void)
{
    /*
     * Kiểm tra RXNE
     *
     * RXNE = 1
     * nghĩa là USART đã nhận được 1 byte.
     */

    if (USART1->SR & (1 << 5))
    {
        char c;

        /*
         * Đọc DR để lấy ký tự.
         *
         * Đồng thời RXNE được clear.
         */

        c = USART1->DR;


        /*
         * ----------------------------------------------------
         * Nếu nhận được !
         *
         * -> kết thúc một lệnh
         * ----------------------------------------------------
         */

        if (c == '!')
        {
            /*
             * Kết thúc chuỗi.
             */

            uart_buffer[uart_index] = '\0';

            /*
             * Xử lý lệnh.
             */

            Process_Command((char *)uart_buffer);

            /*
             * Chuẩn bị nhận lệnh tiếp theo.
             */

            uart_index = 0;
        }


        /*
         * ----------------------------------------------------
         * Nếu chưa gặp !
         * thì lưu ký tự vào buffer.
         * ----------------------------------------------------
         */

        else
        {
            if (uart_index < sizeof(uart_buffer) - 1)
            {
                uart_buffer[uart_index] = c;
                uart_index++;
            }
            else
            {
                /*
                 * Buffer đầy -> reset.
                 */

                uart_index = 0;
            }
        }
    }
}


/* ============================================================
 * GPIO + PWM TIMER
 * ============================================================
 */

void TIM2_PWM_Init(void)
{
    /*
     * --------------------------------------------------------
     * CLOCK GPIOA
     * --------------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 2);


    /*
     * --------------------------------------------------------
     * CLOCK TIM2
     * --------------------------------------------------------
     */

    RCC->APB1ENR |= (1 << 0);


    /*
     * --------------------------------------------------------
     * PA0 = TIM2_CH1
     *
     * Alternate Function Push-Pull
     *
     * MODE = 11 -> 50 MHz
     * CNF  = 10 -> AF Push-Pull
     *
     * 1011 = 0xB
     * --------------------------------------------------------
     */

    GPIOA->CRL &= ~(0xF << 0);
    GPIOA->CRL |=  (0xB << 0);


    /*
     * --------------------------------------------------------
     * TIMER
     *
     * TIM2 clock = 72 MHz
     *
     * PSC = 71
     *
     * 72 MHz / 72 = 1 MHz
     *
     * 1 count = 1 us
     * --------------------------------------------------------
     */

    TIM2->PSC = 71;


    /*
     * --------------------------------------------------------
     * PWM = 1 kHz
     *
     * T = 1 ms
     *
     * ARR = 999
     * --------------------------------------------------------
     */

    TIM2->ARR = 999;


    /*
     * --------------------------------------------------------
     * PWM ban đầu = 50%
     *
     * Nhưng LED ban đầu OFF.
     *
     * Vì vậy:
     *
     * pwm_percent = 50
     * CCR1 = 0
     *
     * Khi ON:
     *
     * CCR1 = 500
     * --------------------------------------------------------
     */

    pwm_percent = 50;
    led_state = 0;

    TIM2->CCR1 = 0;


    /*
     * --------------------------------------------------------
     * CH1:
     *
     * OC1PE = 1
     * OC1M = 110 -> PWM Mode 1
     * --------------------------------------------------------
     */

    TIM2->CCMR1 = 0;

    TIM2->CCMR1 |= (1 << 3);

    TIM2->CCMR1 |= (6 << 4);


    /*
     * --------------------------------------------------------
     * Enable CH1
     * --------------------------------------------------------
     */

    TIM2->CCER = 0;

    TIM2->CCER |= (1 << 0);


    /*
     * ARR preload
     */

    TIM2->CR1 |= (1 << 7);


    /*
     * Update Event
     */

    TIM2->EGR |= (1 << 0);


    /*
     * Start Timer
     */

    TIM2->CR1 |= (1 << 0);
}


/* ============================================================
 * USART1 INIT
 * ============================================================
 */

void USART1_Init(void)
{
    /*
     * --------------------------------------------------------
     * CLOCK GPIOA
     * --------------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 2);


    /*
     * --------------------------------------------------------
     * CLOCK USART1
     * --------------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 14);


    /*
     * --------------------------------------------------------
     * PA9 = USART1_TX
     *
     * Alternate Function Push-Pull
     *
     * MODE = 11
     * CNF = 10
     *
     * -> 0xB
     * --------------------------------------------------------
     */

    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);


    /*
     * --------------------------------------------------------
     * PA10 = USART1_RX
     *
     * Input Floating
     *
     * -> 0x4
     * --------------------------------------------------------
     */

    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);


    /*
     * --------------------------------------------------------
     * BAUD RATE
     *
     * USART1 clock = 72 MHz
     *
     * Baud = 9600
     *
     * BRR = 7500
     * --------------------------------------------------------
     */

    USART1->BRR = 7500;


    /*
     * --------------------------------------------------------
     * CR1
     *
     * UE  = bit 13 -> USART enable
     * RXNEIE = bit 5 -> RX interrupt enable
     * TE  = bit 3 -> transmitter enable
     * RE  = bit 2 -> receiver enable
     * --------------------------------------------------------
     */

    USART1->CR1 = 0;

    USART1->CR1 |= (1 << 13);
    USART1->CR1 |= (1 << 5);
    USART1->CR1 |= (1 << 3);
    USART1->CR1 |= (1 << 2);


    /*
     * --------------------------------------------------------
     * ENABLE USART1 INTERRUPT IN NVIC
     *
     * USART1 IRQ number = 37
     *
     * ISER[1], bit 5
     * --------------------------------------------------------
     */

    NVIC->ISER[1] |= (1 << 5);
}


/* ============================================================
 * MAIN
 * ============================================================
 */

int main(void)
{
    /*
     * Khởi tạo PWM
     */

    TIM2_PWM_Init();


    /*
     * Khởi tạo UART
     */

    USART1_Init();


    /*
     * Chương trình chính không cần polling UART.
     *
     * UART nhận bằng interrupt.
     */

    while (1)
    {
    }
}