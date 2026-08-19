/*
 * timer.c
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */
#include "timer.h"
#include "stm32f411_reg.h"
#include <stdint.h>
#define SERVO_MIN_PULSE_US 1000
#define SERVO_MAX_PULSE_US 2000

void TIM2_init(void)
{
    RCC->APB1ENR |= (1 << 0);
    TIM2->PSC = 16 - 1; /* 16 MHz / 16 = 1 MHz, or 1 us per count. */
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->EGR |= (1 << 0);
    TIM2->CNT = 0;
    TIM2->CR1 |= (1 << 0);
}

void TIM2_delayus(uint32_t us)
{
    uint32_t start = TIM2->CNT;
    while ((TIM2->CNT - start) < us) {
    }
}

void TIM4_PWM_PB6_init(void)
{
    RCC->AHB1ENR |= (1 << 1);
    RCC->APB1ENR |= (1 << 2);

    TIM4->CR1 &= ~(1 << 0);

    GPIOB->MODER &= ~(3 << (6 * 2));
    GPIOB->MODER |=  (2 << (6 * 2));

    GPIOB->AFR[0] &= ~(0xF << (6 * 4));
    GPIOB->AFR[0] |=  (2 << (6 * 4));

    GPIOB->OTYPER &= ~(1 << 6);

    GPIOB->OSPEEDR &= ~(3 << (6 * 2));
    GPIOB->OSPEEDR |=  (1 << (6 * 2));

    GPIOB->PUPDR &= ~(3 << (6 * 2));

    TIM4->PSC = 16 - 1;
    TIM4->ARR = 20000 - 1;
    TIM4->CCR1 = 1500;

    /* PWM mode 1: PB6 is high while CNT is below CCR1. */
    TIM4->CCMR1 &= ~((3 << 0) | (7 << 4));
    TIM4->CCMR1 |=  (6 << 4);
    TIM4->CCMR1 |=  (1 << 3);

    TIM4->CCER &= ~(1 << 1);
    TIM4->CCER |=  (1 << 0);

    TIM4->CR1 |= (1 << 7);

    /* Load prescaler and preload values before enabling the counter. */
    TIM4->EGR |= (1 << 0);

    TIM4->CR1 |= (1 << 0);
}

void Servo_SetPulseUs(uint32_t pulse_us)
{
	if (pulse_us < SERVO_MIN_PULSE_US) {
	    pulse_us = SERVO_MIN_PULSE_US;
	}

	if (pulse_us > SERVO_MAX_PULSE_US) {
		pulse_us = SERVO_MAX_PULSE_US;
	}

	TIM4->CCR1 = pulse_us;
}

void Servo_SetAngle(uint32_t angle)
{
    if (angle > 180) {
        angle = 180;
    }

    uint32_t pulse_us = SERVO_MIN_PULSE_US
                      + (angle * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) / 180;

    Servo_SetPulseUs(pulse_us);
}
