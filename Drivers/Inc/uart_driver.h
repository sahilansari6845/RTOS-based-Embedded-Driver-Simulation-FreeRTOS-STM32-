#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "stm32_regmap.h"
#include <stddef.h>

void UART_Init(USART_TypeDef *huart, uint32_t baud);

/* Blocking-style API (as used from FreeRTOS task context) */
void   UART_SendChar(USART_TypeDef *huart, char c);
void   UART_SendString(USART_TypeDef *huart, const char *str);

/* Called from the "ISR simulation" thread when a byte arrives on the wire */
void   UART_RxIRQHandler(USART_TypeDef *huart, char received);

/* Task-side: blocks on the internal RX queue until a byte is available */
int    UART_ReceiveChar(USART_TypeDef *huart, char *out, uint32_t timeout_ticks);

#endif /* UART_DRIVER_H */
