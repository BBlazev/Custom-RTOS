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

