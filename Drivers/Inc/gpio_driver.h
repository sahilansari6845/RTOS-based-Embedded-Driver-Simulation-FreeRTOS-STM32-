#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32_regmap.h"

typedef enum {
    GPIO_MODE_INPUT  = 0,
    GPIO_MODE_OUTPUT = 1
} GPIO_Mode_t;

typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET   = 1
} GPIO_PinState_t;

void            GPIO_Init(GPIO_TypeDef *port, uint32_t pin, GPIO_Mode_t mode);
void            GPIO_WritePin(GPIO_TypeDef *port, uint32_t pin, GPIO_PinState_t state);
void            GPIO_TogglePin(GPIO_TypeDef *port, uint32_t pin);
GPIO_PinState_t GPIO_ReadPin(GPIO_TypeDef *port, uint32_t pin);

#endif /* GPIO_DRIVER_H */
