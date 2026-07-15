/**
 * @file  uart_driver.c
 * @brief Interrupt-style UART driver.
 *
 * On real hardware, UART_RxIRQHandler() would be called from the NVIC
 * USART2_IRQHandler ISR when the RXNE flag is set. In this simulation the
 * "hardware_sim" module plays that role by feeding bytes in on a timer,
 * exactly the way a real UART peripheral raises interrupts. A FreeRTOS
 * queue acts as the RX ring buffer -- this is the standard, textbook way
 * to hand data from an ISR to a task safely (xQueueSendFromISR).
 */
#include "uart_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

#define UART_RX_QUEUE_LEN  64

static QueueHandle_t xUartRxQueue = NULL;

void UART_Init(USART_TypeDef *huart, uint32_t baud)
{
    huart->BRR = baud;
    huart->CR1 = 1;              /* UE: USART enable            */
    huart->SR  = USART_SR_TXE;   /* TX register starts empty     */

    if (xUartRxQueue == NULL) {
        xUartRxQueue = xQueueCreate(UART_RX_QUEUE_LEN, sizeof(char));
    }
}

void UART_SendChar(USART_TypeDef *huart, char c)
{
    /* Real driver would poll/wait on TXE before writing DR; register
     * state is updated the same way here for fidelity. */
    huart->DR = (uint32_t)c;
    huart->SR |= USART_SR_TXE;
    putchar(c);          /* the "wire": simulated UART TX -> host stdout */
    fflush(stdout);
}

void UART_SendString(USART_TypeDef *huart, const char *str)
{
    while (*str) {
        UART_SendChar(huart, *str++);
    }
}

/* Simulated ISR entry point: called from the hardware-sim thread */
void UART_RxIRQHandler(USART_TypeDef *huart, char received)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    huart->DR = (uint32_t)received;
    huart->SR |= USART_SR_RXNE;

    if (xUartRxQueue != NULL) {
        xQueueSendFromISR(xUartRxQueue, &received, &xHigherPriorityTaskWoken);
    }
    /* On real hardware: portYIELD_FROM_ISR(xHigherPriorityTaskWoken); */
}

int UART_ReceiveChar(USART_TypeDef *huart, char *out, uint32_t timeout_ticks)
{
    (void)huart;
    if (xUartRxQueue == NULL) return 0;
    return xQueueReceive(xUartRxQueue, out, timeout_ticks) == pdTRUE;
}
