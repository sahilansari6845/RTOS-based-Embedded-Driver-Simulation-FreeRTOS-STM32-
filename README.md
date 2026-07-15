# RTOS-based Embedded Driver Simulation (FreeRTOS + STM32)

A host-runnable simulation of an STM32 firmware stack running under
**FreeRTOS**, built without needing any physical board. It uses the real,
official **FreeRTOS-Kernel** source (the same kernel you'd flash onto real
silicon) with its **GCC/POSIX simulator port**, so every FreeRTOS task runs
as one pthread on your Linux machine, while the driver code is written
exactly the way you'd write it for a real STM32F4.

## Why "simulation"?

STM32 peripherals are normally accessed through fixed memory addresses
(e.g. `GPIOA` at `0x40020000`). Here, each peripheral is a plain C struct
with the exact same register layout (`MODER`, `ODR`, `BSRR`, `DR`, `SR`,
`CR1`, `CR2`...). Driver code reads and writes these registers with the
same bit-level logic used on real hardware — only the backing memory
(a struct instead of a memory-mapped address) is different. This means the
driver logic (`Drivers/`) is portable: point the `stm32_regmap.h` macros at
real addresses and it becomes real STM32 firmware.

## Architecture

```
                        ┌─────────────────────────┐
                        │      hardware_sim.c       │ ← simulates the
                        │ (fake UART bytes/ button) │   outside world
                        └────────────┬───────────--─┘
                                     │ calls into drivers
                                     ▼
   ┌──────────────────────────────────────────────────────────────┐
   │                     FreeRTOS Scheduler (Posix port)            │
   │                                                                │
   │  LED_Task   BTN_Task   UART_RX   ADC_Task  Logger  SysMon     │
   │     │           │          │         │        │       │       │
   └─────┼───────────┼──────────┼─────────┼────────┼───────┼───────┘
         ▼           ▼          ▼         ▼        ▼       ▼
   ┌──────────────────────────────────────────────────────────────┐
   │                Drivers/  (GPIO, UART, ADC)                     │
   │       register-level access to simulated peripherals          │
   └──────────────────────────────────────────────────────────────┘
         ▼           ▼          ▼
   ┌──────────────────────────────────────────────────────────────┐
   │        stm32_regmap.c/.h  -- simulated peripheral "silicon"    │
   └──────────────────────────────────────────────────────────────┘
```

## Directory layout

```
rtos_project/
├── FreeRTOS-Kernel/          Official FreeRTOS kernel (cloned from GitHub)
│   └── portable/ThirdParty/GCC/Posix/   ← simulator port used for this build
├── Config/
│   └── FreeRTOSConfig.h      Kernel configuration for the POSIX port
├── Drivers/
│   ├── Inc/                  Driver + register-map headers
│   │   ├── stm32_regmap.h    Simulated GPIO/USART/ADC/TIM register structs
│   │   ├── gpio_driver.h
│   │   ├── uart_driver.h
│   │   └── adc_driver.h
│   └── Src/
│       ├── stm32_regmap.c    Peripheral instance storage
│       ├── gpio_driver.c     Bit-level GPIO driver (MODER/BSRR/IDR)
│       ├── uart_driver.c     Interrupt-style UART driver (RX queue, TX)
│       └── adc_driver.c      Polled ADC driver (simulated analog signal)
├── App/
│   ├── hardware_sim.c/.h     Injects UART bytes + button presses
│   └── main.c                Creates tasks, queues, mutex, semaphore
├── Makefile
└── README.md
```

## What it demonstrates

| Concept                       | Where                                                |
|--------------------------------|-------------------------------------------------------|
| GPIO driver (output + input)   | `gpio_driver.c` — LED blink task, button input         |
| Interrupt-driven UART RX       | `uart_driver.c` — `UART_RxIRQHandler` + FreeRTOS queue |
| Polled ADC sampling            | `adc_driver.c` — simulated sensor voltage              |
| Mutex (shared resource)        | `xUartMutex` — serializes UART TX across tasks         |
| Queue (task-to-task signaling) | `xButtonEventQueue` — button task → LED task           |
| Binary semaphore (producer/consumer) | `xAdcReadySem` — ADC task → data-logger task    |
| System diagnostics task        | `vTask_SystemMonitor` — free heap / task count          |
| Simulated external stimuli     | `hardware_sim.c` — fake serial input + button presses  |

## Build & run

Requires `gcc` and `make` (Linux; pthread support). No STM32 toolchain or
board needed.

```bash
make        # builds FreeRTOS kernel + drivers + app
make run    # or: ./build/rtos_driver_sim
```

Press `Ctrl+C` to stop (it runs forever, like real firmware).

Sample output:

```
[LED       ] ON
[ADC       ] raw=2129  voltage=1.72V
[SYSMON    ] Free heap: 757160 bytes | Active tasks: 9 | Uptime: 1 ticks
[UART-RX   ] RX cmd: "PING" -> ACK
[LED       ] OFF
[BUTTON    ] Press detected -> event queued
[LED       ] Fast-blink mode (button pressed)
[LOGGER    ] Logged 5 ADC samples so far
```

## Porting to real STM32 hardware

To move this onto an actual STM32F4 Discovery/Nucleo board:

1. Replace `Config/FreeRTOSConfig.h`'s POSIX-specific bits with the
   Cortex-M4 config (tick via SysTick, `configCPU_CLOCK_HZ` = your real
   clock).
2. Swap the kernel port from `portable/ThirdParty/GCC/Posix` to
   `portable/GCC/ARM_CM4F`.
3. In `stm32_regmap.h`, change the peripheral macros from struct instances
   to real memory addresses, e.g.:
   ```c
   #define GPIOA  ((GPIO_TypeDef *)0x40020000)
   ```
   (or simply `#include "stm32f4xx.h"` from CMSIS instead of this file).
4. Replace `hardware_sim.c` with real NVIC interrupt handlers
   (`USART2_IRQHandler`, `EXTI15_10_IRQHandler`) calling the same
   `UART_RxIRQHandler()` function.
5. `Drivers/` and `App/main.c` need **no changes** — that's the point of
   writing drivers against a register-map abstraction.

## Notes

- `ADC_ReadChannel()` simulates a drifting sensor (sine + noise) since
  there's no real analog input to sample.
- `hardware_sim.c` "types" `PING` / `STATUS` / `HELLO` over UART every ~4s
  and pulses the button every ~2.5s, standing in for real external events.
