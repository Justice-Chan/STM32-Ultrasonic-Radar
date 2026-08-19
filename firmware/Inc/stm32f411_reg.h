/*
 * stm32f411_reg.h
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */

#ifndef STM32F411_REG_H_
#define STM32F411_REG_H_
#include <stdint.h>
// -- OFFSET STRUCT DEFI: RCC--
typedef struct {
	volatile uint32_t CR;
	volatile uint32_t PLLCFGR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t AHB1RSTR;
	volatile uint32_t AHB2RSTR;
	uint32_t RESERVED0[2];
	volatile uint32_t APB1RSTR;
	volatile uint32_t APB2RSTR;
	uint32_t RESERVED1[2];
	volatile uint32_t AHB1ENR;
	volatile uint32_t AHB2ENR;
	uint32_t RESERVED2[2];
	volatile uint32_t APB1ENR;
	volatile uint32_t APB2ENR;
	uint32_t RESERVED3[2];
	volatile uint32_t AHB1LPENR;
	volatile uint32_t AHB2LPENR;
	uint32_t RESERVED4[2];
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
	uint32_t RESERVED5[2];
	volatile uint32_t SSCGR;
	volatile uint32_t PLLI2SCFGR;
	volatile uint32_t DCKCFGR;
} RCC_FUNC;

typedef struct {
	volatile uint32_t MODER;
	volatile uint32_t OTYPER;
	volatile uint32_t OSPEEDR;
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFR[2];
} GPIO_FUNC;

typedef struct {
	volatile uint32_t SR; // Shift Register(7th bit TXE(flag))檢查DR是否為空
	volatile uint32_t DR; // Data Register (8 bit capacity)
	volatile uint32_t BRR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t CR3;
	volatile uint32_t GTPR;
} USART2_FUNC;

typedef struct {
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SMCR;
	volatile uint32_t DIER;
	volatile uint32_t SR;
	volatile uint32_t EGR;
	volatile uint32_t CCMR1;
	volatile uint32_t CCMR2;
	volatile uint32_t CCER;
	volatile uint32_t CNT;
	volatile uint32_t PSC;
	volatile uint32_t ARR;
	uint32_t RESERVED0;
	volatile uint32_t CCR1;
	volatile uint32_t CCR2;
	volatile uint32_t CCR3;
	volatile uint32_t CCR4;
	uint32_t RESERVED1;
	volatile uint32_t DCR;
	volatile uint32_t DMAR;
	volatile uint32_t OR;
} TIMx_FUNC;

typedef struct {
	volatile uint32_t MEMRMP;
	volatile uint32_t PMC;
	volatile uint32_t EXTICR[4];
	volatile uint32_t CMPCR;
} SYSCFG_FUNC;

typedef struct {
	volatile uint32_t IMR;
	volatile uint32_t EMR;
	volatile uint32_t RTSR;
	volatile uint32_t FTSR;
	volatile uint32_t SWIER;
	volatile uint32_t PR;
} EXTI_FUNC;

typedef struct {
	volatile uint32_t ISER[8]; // Offset: 0x000 (每 32 個中斷一個暫存器，STM32F411 最多支援到 80 幾個中斷)
	uint32_t RESERVED0[24];     // 填補記憶體空隙
	volatile uint32_t ICER[8];  // Offset: 0x080 (Clear-Enable Register, 關閉中斷用)
// 後面還有 ISPR, ICPR, IABR, IPR 等暫存器
} NVIC_FUNC;

typedef struct {
	volatile uint32_t LISR;   // Low Interrupt Status
	volatile uint32_t HISR;   // High Interrupt Status
	volatile uint32_t LIFCR;  // Low Interrupt Flag Clear
	volatile uint32_t HIFCR;  // High Interrupt Flag Clear
	// 每個 DMA 有 8 個 Stream (0~7)
	struct {
		volatile uint32_t CR;   // Control Register
		volatile uint32_t NDTR; // Number of Data Register (要搬幾個 Byte)
		volatile uint32_t PAR;  // Peripheral Address Register (周邊位址，如 UART->DR)
		volatile uint32_t M0AR; // Memory 0 Address Register (記憶體位址，如字串陣列)
		volatile uint32_t M1AR; // Memory 1 Address Register (Double buffer模式使用)
		volatile uint32_t FCR;  // FIFO Control Register
	} S[8];
} DMA_FUNC;

typedef struct {
	volatile uint32_t CTRL;
	volatile uint32_t LOAD;
	volatile uint32_t VAL;
} STK_FUNC;

// -- BASE ADDR & Structure Beginning ADDR --
#define RCC_BASE_ADDR 0x40023800
#define GPIOA_BASE_ADDR 0x40020000
#define GPIOB_BASE_ADDR 0x40020400
#define USART2_BASE_ADDR 0x40004400
#define TIM2_BASE_ADDR 0x40000000
#define TIM4_BASE_ADDR 0x40000800
#define SYSCFG_BASE_ADDR 0x40013800
#define EXTI_BASE_ADDR 0x40013C00
// -- Arm Holding(Not in RM0308) --
#define NVIC_BASE_ADDR 0xE000E100
#define DMA1_BASE_ADDR 0x40026000
#define STK_BASE_ADDR 0xE000E010


#define RCC ((RCC_FUNC *) RCC_BASE_ADDR)
#define GPIOA ((GPIO_FUNC *) GPIOA_BASE_ADDR)
#define GPIOB ((GPIO_FUNC *) GPIOB_BASE_ADDR)
#define USART2 ((USART2_FUNC *) USART2_BASE_ADDR) // 4 bit fraction + 12 bit mantissa
#define TIM2 ((TIMx_FUNC *) TIM2_BASE_ADDR)
#define TIM4 ((TIMx_FUNC *) TIM4_BASE_ADDR)
#define SYSCFG ((SYSCFG_FUNC *) SYSCFG_BASE_ADDR)
#define EXTI ((EXTI_FUNC *) EXTI_BASE_ADDR)
#define NVIC ((NVIC_FUNC *) NVIC_BASE_ADDR)
#define DMA1 ((DMA_FUNC *) DMA1_BASE_ADDR)
#define STK ((STK_FUNC *) STK_BASE_ADDR)

#endif /* STM32F411_REG_H_ */
