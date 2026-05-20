.syntax unified
.cpu cortex-m3
.thumb

.section .text
.align 2

.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func

PendSV_Handler:
    mrs r0, psp

    stmdb r0!, {r4-r11}


    ldr r1, =g_current_task
    ldr r1, [r1]
    str r0, [r1]

    ldr r1, =g_next_task
    ldr r1, [r1]
    ldr r2, =g_current_task
    str r1, [r2]

    ldr r0, [r1]

    ldmia r0!, {r4-r11}

    msr psp, r0

    bx lr

    .size PendSV_Handler, .-PendSV_Handler