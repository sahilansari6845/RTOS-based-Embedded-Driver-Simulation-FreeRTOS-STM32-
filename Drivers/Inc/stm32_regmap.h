/**
 ******************************************************************************
 * @file    stm32_regmap.h
 * @brief   Simulated STM32F4-style peripheral register map.
 *
 * Since this project runs as a host-side simulation (no physical STM32
 * silicon attached), every "peripheral" below is a plain RAM-backed struct
 * that mimics the real CMSIS register layout (GPIO_TypeDef, USART_TypeDef,
 * ADC_TypeDef, TIM_TypeDef). Driver code reads/writes these registers
 * exactly the way it would on real hardware (->MODER, ->ODR, ->DR, etc.),
 * so the driver logic you write here is structurally identical to what you
 * would flash onto a real STM32F4 Discovery/Nucleo board. Only the memory
 * backing (RAM struct vs. memory-mapped 0x40020000 address) differs.
 ******************************************************************************
 */
#ifndef STM32_REGMAP_H
#define STM32_REGMAP_H

#include <stdint.h>

/* ---------------------------------------------------------------------- */
/*  GPIO (simplified, mirrors STM32F4 GPIO_TypeDef)                        */
/* ---------------------------------------------------------------------- */
typedef struct {
    volatile uint32_t MODER;   /* Mode register            */
    volatile uint32_t OTYPER;  /* Output type register      */
    volatile uint32_t OSPEEDR; /* Output speed register     */
    volatile uint32_t PUPDR;   /* Pull-up/pull-down         */
    volatile uint32_t IDR;     /* Input data register       */
    volatile uint32_t ODR;     /* Output data register      */
    volatile uint32_t BSRR;    /* Bit set/reset register    */
} GPIO_TypeDef;

/* ---------------------------------------------------------------------- */
/*  USART (simplified, mirrors STM32F4 USART_TypeDef)                      */
/* ---------------------------------------------------------------------- */
typedef struct {
    volatile uint32_t SR;      /* Status register           */
    volatile uint32_t DR;      /* Data register (TX/RX)     */
    volatile uint32_t BRR;     /* Baud rate register        */
    volatile uint32_t CR1;     /* Control register 1        */
} USART_TypeDef;

#define USART_SR_TXE   (1U << 7)   /* Transmit data reg empty */
#define USART_SR_RXNE  (1U << 5)   /* Read data reg not empty */

/* ---------------------------------------------------------------------- */
/*  ADC (simplified, mirrors STM32F4 ADC_TypeDef)                          */
/* ---------------------------------------------------------------------- */
typedef struct {
    volatile uint32_t SR;      /* Status register           */
    volatile uint32_t CR1;     /* Control register 1        */
    volatile uint32_t CR2;     /* Control register 2 (ADON) */
    volatile uint32_t DR;      /* Data register (conv result)*/
} ADC_TypeDef;

#define ADC_SR_EOC   (1U << 1)     /* End of conversion */
#define ADC_CR2_ADON (1U << 0)     /* A/D converter ON  */

/* ---------------------------------------------------------------------- */
/*  TIM (simplified general purpose timer)                                 */
/* ---------------------------------------------------------------------- */
typedef struct {
    volatile uint32_t CR1;     /* Control register          */
    volatile uint32_t CNT;     /* Counter value              */
    volatile uint32_t ARR;     /* Auto-reload register       */
    volatile uint32_t SR;      /* Status (UIF flag)          */
} TIM_TypeDef;

#define TIM_SR_UIF  (1U << 0)

/* ---------------------------------------------------------------------- */
/*  "Memory map" - instead of real addresses (0x40020000 etc.) each        */
/*  peripheral instance is a statically allocated struct, declared extern  */
/*  here and defined once in stm32_regmap.c. This is the only difference   */
/*  vs. real hardware: normally these macros would be                      */
/*      #define GPIOA ((GPIO_TypeDef *)0x40020000)                         */
/* ---------------------------------------------------------------------- */
extern GPIO_TypeDef  SIM_GPIOA;
extern USART_TypeDef SIM_USART2;
extern ADC_TypeDef   SIM_ADC1;
extern TIM_TypeDef   SIM_TIM2;

#define GPIOA    (&SIM_GPIOA)
#define USART2   (&SIM_USART2)
#define ADC1     (&SIM_ADC1)
#define TIM2     (&SIM_TIM2)

/* GPIO pin numbers used in this project */
#define GPIO_PIN_5   5   /* On-board LED,  like Nucleo LD2 */
#define GPIO_PIN_13  13  /* User button,   like Nucleo B1  */

#endif /* STM32_REGMAP_H */
