/*
 * scan.c
 *
 *  Created on: 2026年8月14日
 *      Author: justicechan
 */
#include "sr04.h"
#include "stm32f411_reg.h"
#include "timer.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>
typedef enum {
    SCAN_MOVE,
    SCAN_SETTLE,
    SCAN_WAIT_MEASURE
} SCAN_STATE;

#define SCAN_MIN_ANGLE 0
#define SCAN_MAX_ANGLE 180
#define SCAN_SETTLE_TIME_US 150000
#define SCAN_STEP_DEG 5

static SCAN_STATE scan_state = SCAN_MOVE;
static uint32_t scan_timestamp = 0;
static int32_t scan_angle = 0;
static int32_t scan_step = SCAN_STEP_DEG;

static char TX_Buffer[2][80];
static uint32_t cur_write_idx = 0;
static volatile uint8_t tx_pending = 0;

static void SCAN_QueueMeasure(int32_t angle, uint32_t distance)
{
    snprintf(TX_Buffer[cur_write_idx], sizeof(TX_Buffer[cur_write_idx]),
        "Ang:%ld,Dist:%lu\r\n", angle, distance);

    tx_pending = 1;
}

static void SCAN_QueueMove(int32_t angle)
{
    snprintf(TX_Buffer[cur_write_idx], sizeof(TX_Buffer[cur_write_idx]),
        "MOVE:%ld\r\n", angle);

    tx_pending = 1;
}

static void SCAN_TxTask(void)
{
    if (!tx_pending || (DMA1->S[6].CR & (1 << 0))) {
        return;
    }

    DMA_Send_String(TX_Buffer[cur_write_idx]);
    cur_write_idx = 1 - cur_write_idx;
    tx_pending = 0;
}

void SCAN_TASK(void)
{
    SCAN_TxTask();
    if (scan_state == SCAN_MOVE) {
        Servo_SetAngle((uint32_t)scan_angle);
        SCAN_QueueMove(scan_angle);
        scan_timestamp = TIM2->CNT;
        scan_state = SCAN_SETTLE;
    }

    else if (scan_state == SCAN_SETTLE) {
        if ((TIM2->CNT - scan_timestamp) >= SCAN_SETTLE_TIME_US) {
            SR04_StartMeasurement();
            scan_state = SCAN_WAIT_MEASURE;
        }
    }

    else if (scan_state == SCAN_WAIT_MEASURE) {
        if (sr04_data_ready) {
            SCAN_QueueMeasure(scan_angle, sr04_distance);

            sr04_data_ready = 0;

            scan_angle += scan_step;

            if (scan_angle >= SCAN_MAX_ANGLE) {
                scan_angle = SCAN_MAX_ANGLE;
                scan_step = -SCAN_STEP_DEG;
            } else if (scan_angle <= SCAN_MIN_ANGLE) {
                scan_angle = SCAN_MIN_ANGLE;
                scan_step = SCAN_STEP_DEG;
            }

            scan_state = SCAN_MOVE;
        }

        /* A timeout must not stop the rest of the sweep. */
        else if (!SR04_IsBusy()) {
            scan_angle += scan_step;

            if (scan_angle >= SCAN_MAX_ANGLE) {
                scan_angle = SCAN_MAX_ANGLE;
                scan_step = -SCAN_STEP_DEG;
            } else if (scan_angle <= SCAN_MIN_ANGLE) {
                scan_angle = SCAN_MIN_ANGLE;
                scan_step = SCAN_STEP_DEG;
            }

            scan_state = SCAN_MOVE;
        }
    }
}
