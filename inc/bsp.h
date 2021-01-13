/*
 * bsp.h
 *
 *  Created on: Jan 18, 2017
 *      Author: pavelgamov
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define BSP_TICKS_PER_SECOND 1000
#define MINUTE_TICKS (BSP_TICKS_PER_SECOND*60)

typedef enum {

    BSP_Pin_UART_TX,
    BSP_Pin_UART_RX,

	BSP_Pin_PowerButton,

	BSP_Pin_BatteryKey,
	BSP_Pin_SolarKey,

	BSP_Pin_Adc_Vsolar,
    BSP_Pin_Adc_Vbattery,

    BSP_Pin_LED_Red,
    BSP_Pin_LED_Green,

    BSP_Pin_VBATT,

	BSP_Pin_Last,
} BSP_Pin_t;


_Bool BSP_Init(void);
void BSP_InitGpio(void);

void BSP_ShowCharge(unsigned percent);

void BSP_FeedWatchdog(void);

void BSP_SetPinVal(const BSP_Pin_t pin, const _Bool state);
_Bool BSP_GetPinVal(const BSP_Pin_t pin);

#ifdef __cplusplus
}
#endif
