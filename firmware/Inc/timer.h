/*
 * timer.h
 *
 *  Created on: 2026年7月30日
 *      Author: justicechan
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
void TIM2_init(void);
void TIM2_delayus(uint32_t us);
void TIM4_PWM_PB6_init(void);
void Servo_SetPulseUs(uint32_t pulse_us);
void Servo_SetAngle(uint32_t angle);

#endif /* TIMER_H_ */
