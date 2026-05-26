.syntax unified
.cpu cortex-m3
.thumb

.section .text
.align 2

.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func

PendSV_Handler:

    ldr r1, =g_current_task
    ldr r1, [r1]
    cbz r1, restore_only    

    mrs r0, psp
    stmdb r0!, {r4-r11}
    str r0, [r1]

    ldr r1, =g_next_task
    ldr r1, [r1]
    ldr r2, =g_current_task
    str r1, [r2]
    b   restore

restore_only:
    ldr r1, =g_next_task
    ldr r1, [r1]
    ldr r2, =g_current_task
    str r1, [r2]

restore:
    ldr r0, [r1]
    ldmia r0!, {r4-r11}
    msr psp, r0
    ldr lr, =0xFFFFFFFD
    bx lr

    .size PendSV_Handler, .-PendSV_Handler