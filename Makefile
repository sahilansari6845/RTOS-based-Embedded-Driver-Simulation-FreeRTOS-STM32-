# ---------------------------------------------------------------------------
# RTOS-based Embedded Driver Simulation (FreeRTOS + STM32)
# Builds the project as a native Linux binary using the FreeRTOS POSIX
# simulator port. Each FreeRTOS task becomes one pthread.
# ---------------------------------------------------------------------------

KERNEL_DIR   := FreeRTOS-Kernel
PORT_DIR     := $(KERNEL_DIR)/portable/ThirdParty/GCC/Posix
MEMMANG_DIR  := $(KERNEL_DIR)/portable/MemMang

CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -g -pthread \
            -I Config \
            -I Drivers/Inc \
            -I App \
            -I $(KERNEL_DIR)/include \
            -I $(PORT_DIR) \
            -I $(PORT_DIR)/utils
LDFLAGS  := -pthread -lm

SRCS := \
    $(KERNEL_DIR)/tasks.c \
    $(KERNEL_DIR)/queue.c \
    $(KERNEL_DIR)/list.c \
    $(KERNEL_DIR)/timers.c \
    $(KERNEL_DIR)/event_groups.c \
    $(KERNEL_DIR)/stream_buffer.c \
    $(MEMMANG_DIR)/heap_4.c \
    $(PORT_DIR)/port.c \
    $(PORT_DIR)/utils/wait_for_event.c \
    Drivers/Src/stm32_regmap.c \
    Drivers/Src/gpio_driver.c \
    Drivers/Src/uart_driver.c \
    Drivers/Src/adc_driver.c \
    App/hardware_sim.c \
    App/main.c

OBJS := $(SRCS:.c=.o)
TARGET := build/rtos_driver_sim

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p build
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
