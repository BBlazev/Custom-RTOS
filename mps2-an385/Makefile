CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
LD      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE    = arm-none-eabi-size

TARGET = rtos
BUILD_DIR = build

CPUFLAGS  = -mcpu=cortex-m3 -mthumb


CFLAGS  = $(CPUFLAGS) -Wall -Wextra -O0 -g3 -ffreestanding -fno-common -Iinclude

ASFLAGS = $(CPUFLAGS) -g3


LDFLAGS = $(CPUFLAGS) -T linker.ld -nostdlib -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

C_SOURCES = $(shell find src -name '*.c')
S_SOURCES = $(shell find src -name '*.s')

C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
S_OBJECTS = $(S_SOURCES:%.s=$(BUILD_DIR)/%.o)
OBJECTS   = $(C_OBJECTS) $(S_OBJECTS)

all: $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

run: $(BUILD_DIR)/$(TARGET).elf
	qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -nographic -kernel $(BUILD_DIR)/$(TARGET).elf

debug: $(BUILD_DIR)/$(TARGET).elf
	qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -nographic -kernel $(BUILD_DIR)/$(TARGET).elf -S -gdb tcp::3333

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run debug clean