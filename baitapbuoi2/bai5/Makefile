

TARGET = firmware

CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# =========================================================
# Compiler flags
# =========================================================

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -O1\
         -g3 \
         -Wall \
         -ffreestanding \
         -fdata-sections \
         -ffunction-sections \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER \
         -Iinc \
         -Icmsis/core

# =========================================================
# Linker flags
# =========================================================

LDFLAGS = -T linker/STM32F103C8TX_FLASH.ld \
          -nostdlib \
          -Wl,--gc-sections \
          -Wl,-Map=build/$(TARGET).map

# =========================================================
# Source files
# =========================================================

SRC = src/main.c \
      lib/stm32f10x_gpio.c \
      lib/stm32f10x_rcc.c \
      lib/system_stm32f10x.c

STARTUP = startup/startup_stm32f103xb.s

# Chuyển:
# src/main.c
# lib/stm32f10x_gpio.c
# ...
#
# thành:
# build/src/main.o
# build/lib/stm32f10x_gpio.o
# ...

OBJ = $(SRC:%.c=build/%.o)

# =========================================================
# Default target
# =========================================================

all: build/$(TARGET).bin


# =========================================================
# Compile .c -> .o
# =========================================================

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# =========================================================
# Compile startup .s -> .o
# =========================================================

build/startup.o: $(STARTUP)
	$(AS) $(CFLAGS) -c $< -o $@


# =========================================================
# Link
# =========================================================

build/$(TARGET).elf: $(OBJ) build/startup.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@


# =========================================================
# ELF -> BIN
# =========================================================

build/$(TARGET).bin: build/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@


# =========================================================
# Flash STM32
# =========================================================

flash: build/$(TARGET).bin
	st-flash write $< 0x08000000


# =========================================================
# Clean
# =========================================================

clean:
	rm -rf build


# =========================================================
# Phony targets
# =========================================================

.PHONY: all flash clean