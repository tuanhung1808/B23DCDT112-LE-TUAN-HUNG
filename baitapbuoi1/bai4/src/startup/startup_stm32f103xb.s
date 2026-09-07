.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global _estack

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word Default_Handler  /* NMI */
    .word Default_Handler  /* HardFault */
    .word Default_Handler  /* MemManage */
    .word Default_Handler  /* BusFault */
    .word Default_Handler  /* UsageFault */
    .word 0                /* Reserved */
    .word 0                /* Reserved */
    .word 0                /* Reserved */
    .word 0                /* Reserved */
    .word Default_Handler  /* SVCall */
    .word Default_Handler  /* Debug Monitor */
    .word 0                /* Reserved */
    .word Default_Handler  /* PendSV */
    .word SysTick_Handler  /* SysTick - Đã sửa ở đây! */

    /* External interrupts */
    .rept 60
    .word Default_Handler
    .endr

.size g_pfnVectors, . - g_pfnVectors

.section .text.Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
    bl main
1:
    b 1b
.size Reset_Handler, . - Reset_Handler

.type Default_Handler, %function
Default_Handler:
    b Default_Handler
.size Default_Handler, . - Default_Handler

/* Khai báo liên kết yếu cho ngắt SysTick */
.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler