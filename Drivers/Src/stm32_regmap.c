#include "stm32_regmap.h"

/* Backing storage for the "memory-mapped" peripherals. On real silicon
 * these addresses are fixed by the SoC; here they are just structs. */
GPIO_TypeDef  SIM_GPIOA  = {0};
USART_TypeDef SIM_USART2 = {0};
ADC_TypeDef   SIM_ADC1   = {0};
TIM_TypeDef   SIM_TIM2   = {0};
