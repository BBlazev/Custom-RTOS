.text - compiled code, read only at runtime
.rodata - read only data, string literl, const globals, lives in flash next to .text
.data - glovals/statics initialized to nonzero, eg int counter = 5;, lives in RAM at runtime, but inital value is on flash
stack - region of RAM the SP points to, grows downward on ARM, we reserve top of RAM for it

1. SET bit N (without disturbing others)
reg |= (1U << N);

2. CLEAR bit N (without disturbing others)
reg &= ~(1U << N);

3. TEST bit N (truthy/falsy)
if (reg & (1U << N)) { ... }

4. TOGGLE bit N
reg ^= (1U << N);

Interrupt for Cortex M 
1. CPU finishes current instruction
2. CPU automatically pushes 8 registers onto current stack: R0, R1, R2, R3, R12, LR, PC ,xPSR   
    - also called exception stack frame
3. CPU looks up address of handler in vector table (NMI in my case)
4. CPU sets LR to a value called EXC_RETURN - so it knows how to return from interupt
5. CPU jumps to that address of handler, handler runs as normal function
6. When handler returns (executing bx lr with EXC_RETURN value in LR), 
    cpu sees special LR value that is exception return, pops back those 8 registers off the stack and resumes original code

R0-R3, R12, LR, PC, xPSR are pushed automatically by the hardware, i dont write this
R0-R3 and R12 can be used freely by called func
LR, PC, and xPSR are CPU state that must be preserved otherwise interupted code can resume
R4-R11 are NOT pushed automatically, if handler uses them it must save and restore them itself, in C its done by compiler

SysTick is countdown timer build into COrtex M chip, it has 4 registers in system region of address space
---------------
0xE000E010  CTRL    Bit 0 = enable timer; Bit 1 = enable interrupt on reload; Bit 2 = clock source select
0xE000E014  LOAD    The value to count down from
0xE000E018  VALT    The current counter value (counts down to 0)
0xE000E01C  CALIB   Calibration info, ignore it
---------------
mps2an385 -QEMU runs cortex m3 at 25 mhz, so on clock ticck is 1/25 000 000 sencond = 40 nanosec
if i want systick to fire every 1 millisec, i need it to count down 25 000 ticks, LOAD = 25000 - 1 = 24999 (we count 0)

NVIC - nested vectored interrupt controller, piece of hardware that sits between peripherals and CPU and decides on which interrupts get through
     - it manages enabling/disabling specific interrupts
     -priority - each interrupt has a priority
     - pending state
     - it has its own pile of MMIO registers

Integer-to-string: extract digits with v % 10, divide by 10, fill a buffer backwards, ASCII conversion: '0' + digit
SysTick math: LOAD = (CPU_HZ / 1000) * ms - 1, -1 is because SysTick counts inclusive of 0, divide first, multiply second, to avoid overflow
SysTick CTRL bits: ENABLE (bit 0) starts the counter, TICKINT (bit 1) enables interrupts on reload, CLKSOURCE (bit 2) picks processor clock vs reference clock

g_ticks is systems monotonic clock, counts ms since boot, used for (later):
    -rtos_sleep
    -timeouts on locks/queues
    -round robin etc.

high addr:   stack[256]  ┌──────────────┐  ← top of stack empty
             stack[255]  │    xPSR      │  ← 0x01000000 - crotex M required
             stack[254]  │     PC       │  ← address of task entry function
             stack[253]  │     LR       │  ← 0xFFFFFFFD - EXC_RETURN value
             stack[252]  │     R12      │  ← 0 dont care
             stack[251]  │     R3       │  ← 0
             stack[250]  │     R2       │  ← 0
             stack[249]  │     R1       │  ← 0
             stack[248]  │     R0       │  ← 0  end of hardware frame
             stack[247]  │     R11      │  ← 0  start of software frame
             stack[246]  │     R10      │  ← 0
             stack[245]  │     R9       │  ← 0
             stack[244]  │     R8       │  ← 0
             stack[243]  │     R7       │  ← 0
             stack[242]  │     R6       │  ← 0
             stack[241]  │     R5       │  ← 0
             stack[240]  │     R4       │  ← 0  ← g_tasks[i].sp points here
             stack[239]  │              │
                         │  rest of     │
                         │   stack,     │
                         │   empty      │
             stack[0]    │              │
low addr:                └──────────────┘

fake initial stack frame - task_create:
    - constructs a 16 word frame at top of new tasks stack that mimics what real context switch
        out produces
    -PC slot hold entry function address
    - save xpsr has "Thumb" (cortex m thing) bit set 0x01000000
    -TCB .sp points at bottom of &stack[TASK_STACK_WORDS - 16]

We create a new task with a fake register frame on its stack. When a switch is triggered (PendSV pended), the CPU finishes its current instruction, then automatically pushes 8 registers (R0-R3, R12, LR, PC, xPSR) onto the current tasks stack via PSP, loads LR with EXC_RETURN, switches the active SP to MSP, and jumps to PendSV_Handler. The handler reads PSP into R0, pushes R4-R11 to extend the saved frame, and stores the resulting R0 into g_current_task->sp. Then it overwrites the g_current_task pointer with g_next_task. Then it reads the new tasks saved SP from its TCB, pops R4-R11at stack into the CPU, writes the resulting R0 back to PSP, and does bx lr. The CPU sees EXC_RETURN in LR, pops the other 8 registers from PSP, loads the popped PC, and starts executing the new task.



bootsrap bug: when first pendsv fires from rtos_start, cpu is on MSP
hardware sets exc_return to 0xFFFFFFF9 (return to msp)
meaning it returns to iself back into rtos start spin loop
fix: forced 0xFFFFFFFD before bx lr in handler

EXC_RETURN values:
0xFFFFFFF9 = return to thread mode, MSP
0xFFFFFFFD = return to thread mode, PSP (our tasks)
0xFFFFFFF1 = return to handler mode (nested exceptions)

race condition emerge with preemption
shared variable (last_printed) read-modify-write across a context switch is unsafe