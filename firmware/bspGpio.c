/*
 * bspGpio.c
 *
 *  Created on: Jan 19, 2017
 *      Author: shapa
 */

#include <stdlib.h>

#include "bsp.h"
#include "stm32f0xx_gpio.h"

typedef struct {
	GPIO_TypeDef *port;
	const GPIO_InitTypeDef setting;
} BspGpioConfig_t;

static const BspGpioConfig_t s_gpioConfig[] = {
    [BSP_Pin_UART_TX] = { GPIOA, { GPIO_Pin_9, GPIO_Mode_AF, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },
    [BSP_Pin_UART_RX] = { GPIOA, { GPIO_Pin_10, GPIO_Mode_AF, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },

	[BSP_Pin_PowerButton] = { GPIOF, { GPIO_Pin_1, GPIO_Mode_IN, GPIO_Speed_Level_3, GPIO_OType_PP,  GPIO_PuPd_DOWN } },
	[BSP_Pin_BatteryKey] = { GPIOA, { GPIO_Pin_5, GPIO_Mode_OUT, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },
	[BSP_Pin_SolarKey] = { GPIOA, { GPIO_Pin_6, GPIO_Mode_OUT, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },

	[BSP_Pin_Adc_Vsolar] = { GPIOA, { GPIO_Pin_7, GPIO_Mode_AN, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },
    [BSP_Pin_Adc_Vbattery] = { GPIOA, { GPIO_Pin_0, GPIO_Mode_AN, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },

    [BSP_Pin_LED_Red] = { GPIOB, { GPIO_Pin_1, GPIO_Mode_OUT, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },
    [BSP_Pin_LED_Green] = { GPIOF, { GPIO_Pin_0, GPIO_Mode_OUT, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },

    [BSP_Pin_VBATT] = { GPIOA, { GPIO_Pin_4, GPIO_Mode_AF, GPIO_Speed_Level_1, GPIO_OType_PP,  GPIO_PuPd_NOPULL} },


};

void BSP_InitGpio(void) {
	static const size_t size = sizeof(s_gpioConfig)/sizeof(*s_gpioConfig);
	for (size_t i = 0; i < size; i++)
		GPIO_Init((GPIO_TypeDef*)s_gpioConfig[i].port, (GPIO_InitTypeDef*)&s_gpioConfig[i].setting);

    BSP_SetPinVal(BSP_Pin_LED_Red, false);
    BSP_SetPinVal(BSP_Pin_LED_Green, false);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_4);

	BSP_SetPinVal(BSP_Pin_BatteryKey, false);
	BSP_SetPinVal(BSP_Pin_SolarKey, false);
}

void BSP_SetPinVal(const BSP_Pin_t pin, const _Bool state) {
	if (pin > BSP_Pin_Last)
	    return;
    if (state)
        s_gpioConfig[pin].port->BSRR = s_gpioConfig[pin].setting.GPIO_Pin;
    else
        s_gpioConfig[pin].port->BRR = s_gpioConfig[pin].setting.GPIO_Pin;
}

_Bool BSP_GetPinVal(const BSP_Pin_t pin) {
	if (pin > BSP_Pin_Last)
		return false;
	return (s_gpioConfig[pin].port->IDR & s_gpioConfig[pin].setting.GPIO_Pin);
}

