.syntax unified
.cpu cortex-m3
.thumb

/* =====================================================
   Các symbol từ C
   ===================================================== */
.global Reset_Handler
.global _estack

.extern SystemInit
.extern SystemCoreClockUpdate
.extern main
.extern USART1_IRQHandler
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss


/* =====================================================
   VECTOR TABLE
   ===================================================== */
.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:

    /* 0 - Initial Stack Pointer */
    .word _estack

    /* 1 - Reset Handler */
    .word Reset_Handler

    /* 2 - NMI */
    .word Default_Handler

    /* 3 - HardFault */
    .word Default_Handler

    /* 4 - MemManage */
    .word Default_Handler

    /* 5 - BusFault */
    .word Default_Handler

    /* 6 - UsageFault */
    .word Default_Handler

    /* 7 - Reserved */
    .word 0

    /* 8 - Reserved */
    .word 0

    /* 9 - Reserved */
    .word 0

    /* 10 - Reserved */
    .word 0

    /* 11 - SVCall */
    .word Default_Handler

    /* 12 - DebugMonitor */
    .word Default_Handler

    /* 13 - Reserved */
    .word 0

    /* 14 - PendSV */
    .word Default_Handler

    /* 15 - SysTick */
   .word Default_Handler


    /* =================================================
       External Interrupts
       STM32F103C8 có các IRQ tiếp theo
       ================================================= */

   /* IRQ 0..36 */
   .rept 37
    .word Default_Handler
    .endr

   /* IRQ 37 - USART1 */
   .word USART1_IRQHandler

   /* IRQ 38..59 */
   .rept 22
   .word Default_Handler
   .endr

.size g_pfnVectors, . - g_pfnVectors


/* =====================================================
   RESET HANDLER
   ===================================================== */
.section .text.Reset_Handler
.type Reset_Handler, %function

Reset_Handler:

    /*
       Cấu hình hệ thống:
       HSI → HSE → PLL
       SYSCLK = 72 MHz
    */
    bl SystemInit
   bl SystemCoreClockUpdate

   ldr r0, =_sdata
   ldr r1, =_edata
   ldr r2, =_sidata
   cmp r0, r1
   beq zero_bss
copy_data:
   ldr r3, [r2], #4
   str r3, [r0], #4
   cmp r0, r1
   bcc copy_data

zero_bss:
   ldr r0, =_sbss
   ldr r1, =_ebss
   movs r2, #0
   cmp r0, r1
   beq call_main
clear_bss:
   str r2, [r0], #4
   cmp r0, r1
   bcc clear_bss

    /*
       Sau khi clock được cấu hình
       thì chạy main()
    */
call_main:
    bl main

    /*
       Nếu main() thoát thì đứng tại đây
    */
1:
    b 1b

.size Reset_Handler, . - Reset_Handler


/* =====================================================
   DEFAULT HANDLER
   ===================================================== */
.section .text.Default_Handler
.type Default_Handler, %function

Default_Handler:

1:
    b 1b

.size Default_Handler, . - Default_Handler
