/*
 * sr04.h
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */

#ifndef SR04_H_
#define SR04_H_
#include <stdint.h>

/* Published by the EXTI ISR and consumed by the main-loop scan task. */
extern volatile uint32_t sr04_distance;
extern volatile uint8_t sr04_data_ready;

void SR04_StartMeasurement(void);
void SR04_init(void);
void SR04_Trigger(void);
uint8_t SR04_IsIdle(void);
void SR04_TASK(void);
void EXTI9_5_IRQHandler(void);

#endif /* SR04_H_ */
