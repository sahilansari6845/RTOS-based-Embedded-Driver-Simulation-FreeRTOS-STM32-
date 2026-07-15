/**
 * @file  adc_driver.c
 * @brief Polled single-conversion ADC driver (mirrors STM32 ADC1 12-bit SAR
 *        ADC start/poll/read sequence: set ADON -> start conv -> poll EOC
 *        -> read DR). The "analog signal" being sampled is simulated with a
 *        drifting sine + noise model so repeated reads look like a real
 *        temperature/potentiometer sensor.
 */
#include "adc_driver.h"
#include <math.h>
#include <stdlib.h>

static double phase = 0.0;

void ADC_Init(ADC_TypeDef *hadc)
{
    hadc->CR2 |= ADC_CR2_ADON;   /* power up the ADC   */
    hadc->SR   = 0;
}

uint16_t ADC_ReadChannel(ADC_TypeDef *hadc)
{
    /* --- simulate the analog world (would be real voltage on real HW) --- */
    phase += 0.15;
    double simulated_voltage = 1.65 + 0.5 * sin(phase);              /* 1.15V..2.15V */
    simulated_voltage += ((double)(rand() % 21 - 10)) / 1000.0;       /* +/-10mV noise */

    /* --- simulate the conversion sequence a real driver performs --- */
    hadc->CR2 |= (1U << 30);              /* pretend SWSTART bit                */
    uint16_t raw = (uint16_t)((simulated_voltage / 3.3) * 4095.0);    /* 12-bit  */
    hadc->DR   = raw;
    hadc->SR  |= ADC_SR_EOC;               /* end-of-conversion flag             */

    return raw;
}
