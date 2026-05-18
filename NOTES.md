.text - compiled code, read only at runtime
.rodata - read only data, string literl, const globals, lives in flash next to .text
.data - glovals/statics initialized to nonzero, eg int counter = 5;, lives in RAM at runtime, but inital value is on flash
stack - region of RAM the SP points to, grows downward on ARM, we reserve top of RAM for it



