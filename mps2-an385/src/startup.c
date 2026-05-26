#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;


int main(void);
void Reset_Handler(void);
void Default_Handler(void);


void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector")))
const void * const vector_table[] = {
    (void *)(&_estack),          /*  0: Initial Stack Pointer */
    Reset_Handler,               /*  1: Reset */
    NMI_Handler,                 /*  2: Non-Maskable Interrupt */
    HardFault_Handler,           /*  3: Hard Fault */
    MemManage_Handler,           /*  4: Memory Management Fault */
    BusFault_Handler,            /*  5: Bus Fault */
    UsageFault_Handler,          /*  6: Usage Fault */
    0, 0, 0, 0,                  /*  7-10: Reserved */
    SVC_Handler,                 /* 11: SVCall  */
    DebugMon_Handler,            /* 12: Debug Monitor */
    0,                           /* 13: Reserved */
    PendSV_Handler,              /* 14: PendSV */
    SysTick_Handler,             /* 15: SysTick */
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while(dst < &_edata)
        *dst++ = *src++;

    dst = &_sbss;
    while(dst < &_ebss)
        *dst++ = 0;

    (void)main();

    for(;;){}
}

void Default_Handler(void)
{
    for(;;){}
}