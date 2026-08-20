/*
 * uart.c
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */
#include "uart.h"
#include "stm32f411_reg.h"
#include <stdint.h>
#include <string.h>
#define GPIO_AF7_USART2 7

void USART2_init(void)
{
    RCC->AHB1ENR |= (1 << 0);
    RCC->APB1ENR |= (1 << 17);
    USART2->BRR = (16000000 + (115200 / 2)) / 115200;
    GPIOA->MODER &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER |= (2 << 4) | (2 << 6);
    GPIOA->AFR[0] &= ~((15 << 8) | (15 << 12));
    GPIOA->AFR[0] |= (GPIO_AF7_USART2 << 8) | (GPIO_AF7_USART2 << 12);
    USART2->CR1 = (1 << 2) | (1 << 3) | (1 << 13);
}

void USART2_DMA_init(void)
{
    RCC->AHB1ENR |= (1 << 21);
    USART2->CR3 |= (1 << 7);

    /* DMA1 Stream 6, Channel 4 is the USART2_TX mapping. */
    DMA1->S[6].CR &= ~(1 << 0);
    while (DMA1->S[6].CR & (1 << 0)) {
    }
    DMA1->HIFCR |= (1 << 16) | (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    DMA1->S[6].PAR = (uint32_t)&(USART2->DR);

    /* Channel 4, memory increment, memory-to-peripheral direction. */
    DMA1->S[6].CR |= (4 << 25) | (1 << 10) | (1 << 6);
}

void DMA_Send_String(char* str)
{
    if (DMA1->S[6].CR & (1 << 0)) {
        return;
    }

    /* Clear all Stream 6 status flags before starting another transfer. */
    DMA1->HIFCR |= (1 << 16) | (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    DMA1->S[6].M0AR = (uint32_t)str;
    DMA1->S[6].NDTR = strlen(str);
    DMA1->S[6].CR |= (1 << 0);
}
