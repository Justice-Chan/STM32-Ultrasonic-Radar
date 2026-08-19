/*
 * sr04.c
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */
#include "sr04.h"
#include "stm32f411_reg.h"
#include "timer.h"
#include <stdint.h>

volatile uint32_t sr04_distance = 0;
volatile uint8_t sr04_data_ready = 0;
static uint32_t start_time = 0;
static uint32_t trigger_time = 0;

typedef enum {
    SR04_IDLE,
    SR04_WAIT_RISE,
    SR04_WAIT_FALL
} SR04_STATE;
static volatile SR04_STATE state = SR04_IDLE;

#define SR04_MIN_DURATION_US 100
#define SR04_MAX_DURATION_US 25000
#define SR04_TRIGGER_INTERVAL_US 60000
#define SR04_ECHO_TIMEOUT 40000

void SR04_init(void)
{
    RCC->AHB1ENR |= (1 << 0);
    GPIOA->MODER &= ~((3 << 8) | (3 << 10));
    GPIOA->MODER |= (1 << 8); /* PA4 output; PA5 input. */
    GPIOA->OTYPER &= ~(1 << 4); /* PA4 push-pull. */
    GPIOA->OSPEEDR &= ~(3 << 8);
    GPIOA->OSPEEDR |= (1 << 8);
    /* Keep PA5 low if the Echo connection is absent or floating. */
    GPIOA->PUPDR &= ~(3 << 10);
    GPIOA->PUPDR |= (2 << 10);

    /* Route PA5 to EXTI5 and enable both pulse edges. */
    RCC->APB2ENR |= (1 << 14);
    SYSCFG->EXTICR[1] &= ~(0xF << 4);
    EXTI->RTSR |= (1 << 5);
    EXTI->FTSR |= (1 << 5);
    EXTI->IMR |= (1 << 5);
    EXTI->PR = (1 << 5);
    NVIC->ISER[0] |= (1 << 23);
}

void SR04_Trigger(void)
{
    GPIOA->ODR |= (1 << 4);
    TIM2_delayus(10);
    GPIOA->ODR &= ~(1 << 4);
}

void SR04_StartMeasurement(void)
{
    if (state == SR04_IDLE) {
        sr04_data_ready = 0;
        SR04_Trigger();
        trigger_time = TIM2->CNT;
        state = SR04_WAIT_RISE;
    }
}

uint8_t SR04_IsBusy(void)
{
    return state != SR04_IDLE;
}

void SR04_TASK(void)
{
    if (state != SR04_IDLE) {
        if ((TIM2->CNT - trigger_time) > SR04_ECHO_TIMEOUT) {
            state = SR04_IDLE;
        }
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1 << 5)) {
        if ((GPIOA->IDR & (1 << 5)) && state == SR04_WAIT_RISE) {
            start_time = TIM2->CNT;
            state = SR04_WAIT_FALL;
        } else if (!(GPIOA->IDR & (1 << 5)) && state == SR04_WAIT_FALL) {
            uint32_t duration = TIM2->CNT - start_time;
            if (duration >= SR04_MIN_DURATION_US
                && duration <= SR04_MAX_DURATION_US) {

                sr04_distance = (duration * 17) / 1000;
                sr04_data_ready = 1;
            }
            state = SR04_IDLE;
        }

        EXTI->PR = (1 << 5);
    }
}
