/**
 * @file  gpio_driver.c
 * @brief Bare-metal style GPIO driver operating directly on register bits.
 *        Logic mirrors the STM32 reference manual: 2 bits per pin in MODER,
 *        BSRR for atomic set/reset, IDR for reads.
 */
#include "gpio_driver.h"

void GPIO_Init(GPIO_TypeDef *port, uint32_t pin, GPIO_Mode_t mode)
{
    /* Clear the 2 mode bits for this pin, then set them (MODER: 00=input, 01=output) */
    port->MODER &= ~(0x3U << (pin * 2));
    port->MODER |= ((uint32_t)mode << (pin * 2));
}

void GPIO_WritePin(GPIO_TypeDef *port, uint32_t pin, GPIO_PinState_t state)
{
    if (state == GPIO_PIN_SET) {
        port->BSRR = (1U << pin);          /* Set bit  (lower half of BSRR) */
        port->ODR  |= (1U << pin);
        /* Push-pull output: IDR mirrors the driven level, same as real HW */
        port->IDR  |= (1U << pin);
    } else {
        port->BSRR = (1U << (pin + 16));   /* Reset bit (upper half of BSRR) */
        port->ODR  &= ~(1U << pin);
        port->IDR  &= ~(1U << pin);
    }
}

void GPIO_TogglePin(GPIO_TypeDef *port, uint32_t pin)
{
    if (port->ODR & (1U << pin)) {
        GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    } else {
        GPIO_WritePin(port, pin, GPIO_PIN_SET);
    }
}

GPIO_PinState_t GPIO_ReadPin(GPIO_TypeDef *port, uint32_t pin)
{
    return (port->IDR & (1U << pin)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}
