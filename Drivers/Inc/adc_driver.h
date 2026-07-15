#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "stm32_regmap.h"

void     ADC_Init(ADC_TypeDef *hadc);
uint16_t ADC_ReadChannel(ADC_TypeDef *hadc);   /* blocking, polled conversion */

#endif /* ADC_DRIVER_H */
