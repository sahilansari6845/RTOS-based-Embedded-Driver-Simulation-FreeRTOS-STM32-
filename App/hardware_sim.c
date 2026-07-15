/**
 * @file  hardware_sim.c
 * @brief Stands in for "the outside world": a person typing on a serial
 *        terminal, and a finger pressing the user button. On real hardware
 *        these events arrive as genuine electrical signals that trigger
 *        NVIC interrupts (USART2_IRQHandler, EXTI15_10_IRQHandler). Here
 *        they are injected from a normal-priority FreeRTOS task so the
 *        rest of the system (drivers + application tasks) can't tell the
 *        difference.
 */
#include "hardware_sim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_regmap.h"
#include "uart_driver.h"
#include "gpio_driver.h"
#include <string.h>

static const char *incoming_commands[] = {
    "PING\r\n",
    "STATUS\r\n",
    "HELLO\r\n"
};

void vTask_HardwareSim(void *pvParameters)
{
    (void)pvParameters;
    uint32_t cmd_index = 0;
    uint32_t tick = 0;

    for (;;) {
        /* Every ~4 seconds, "type" a command into the UART RX line,
         * one interrupt (one byte) at a time - exactly how a real UART
         * peripheral would raise RXNE once per received byte. */
        if (tick % 40 == 0) {
            const char *cmd = incoming_commands[cmd_index % 3];
            cmd_index++;
            for (size_t i = 0; i < strlen(cmd); i++) {
                UART_RxIRQHandler(USART2, cmd[i]);
                vTaskDelay(pdMS_TO_TICKS(20)); /* simulated bit-time between bytes */
            }
        }

        /* Every ~2.5 seconds simulate a user button press pulse on PIN_13 */
        if (tick % 25 == 24) {
            GPIOA->IDR |= (1U << GPIO_PIN_13);   /* button pressed  */
        } else if (tick % 25 == 1) {
            GPIOA->IDR &= ~(1U << GPIO_PIN_13);  /* button released */
        }

        tick++;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
