/*
 * uart.h
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */

#ifndef UART_H_
#define UART_H_

#define GPIO_AF7_USART2 7
void USART2_init(void);
void USART2_DMA_init(void);
void write_char(char c);
void DMA_Send_String(char *str);

#endif /* UART_H_ */
