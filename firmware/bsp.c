/*
 * bsp.c
 *
 *  Created on: May 18, 2016
 *      Author: shapa
 */

#include "bsp.h"
#include "system.h"
#include "systemTimer.h"
#include "Queue.h"
#include "timers.h"

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_iwdg.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_dbgmcu.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"

#include <stddef.h>

#define ADC_TIMEOUT (BSP_TICKS_PER_SECOND/50)
#define PWM_PERIOD 0xFF

static void initialize_RCC(void);
static void initWdt(void);
static void initTimPwm(void);
static void initADC(void);
static void initADC_NVIC(void);
static void initEXTI(void);

static void onAdcTimeout(uint32_t id, void *data);

static inline int adcToVoltsVbatt(int adc);
static inline int adcRdiv(int adc, int rHi, int rLo);

static uint32_t s_adcTimerId = INVALID_HANDLE;

static struct {
    uint32_t timerId;
} s_btn;

static void onBtnTimeout(uint32_t id, void *data) {

    Timer_disarm(id);
    int state = BSP_GetPinVal(BSP_Pin_PowerButton);
    if (!state)
        return;
    EventQueue_Push(EVENT_BUTTON, (void*)state, NULL);
}

_Bool BSP_Init(void) {
	initialize_RCC();
	initWdt();
	initTimPwm();
    BSP_InitGpio();
	System_init(NULL);
	System_setStatus(INFORM_IDLE);

	initADC();
	initEXTI();

	return true;
}

static void initEXTI(void) {
    SYSCFG_DeInit();
    EXTI_DeInit();
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource1);

    static const EXTI_InitTypeDef isr = {
        EXTI_Line1,
        EXTI_Mode_Interrupt,
        EXTI_Trigger_Rising_Falling,
        ENABLE
    };
    EXTI_Init((EXTI_InitTypeDef*)&isr);

    static const NVIC_InitTypeDef nvic = {
        EXTI0_1_IRQn,
        0,
        ENABLE
    };
    NVIC_Init((NVIC_InitTypeDef*)&nvic);

    s_btn.timerId = Timer_new(ADC_TIMEOUT, true, onBtnTimeout, NULL);
}

void EXTI0_1_IRQHandler(void) {
    const int flag = EXTI->PR;
    EXTI->PR = flag;
    if (flag & EXTI_Line1) {
        Timer_rearm(s_btn.timerId);
    }
}

void BSP_ShowCharge(unsigned percent) {
    if (percent > 100)
        percent = 100;
    percent = PWM_PERIOD * percent / 100;
    TIM_SetCompare1(TIM14, percent);
}


void BSP_FeedWatchdog(void) {
	IWDG_ReloadCounter();
}


static void initialize_RCC(void) {

	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_WaitForHSEStartUp();
	RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_DBGMCU, ENABLE);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOF);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
}

static void initTimPwm(void) {
    static const TIM_TimeBaseInitTypeDef tcfg = {
            0x00FF,
            TIM_CounterMode_Up,
            PWM_PERIOD * 1.25,
            TIM_CKD_DIV1,
            0
    };
    TIM_TimeBaseInit(TIM14, (TIM_TimeBaseInitTypeDef*)&tcfg);
    TIM_Cmd(TIM14, ENABLE);

    static const TIM_OCInitTypeDef pwm = {
            TIM_OCMode_PWM1,
            TIM_OutputState_Enable,
            TIM_OutputNState_Disable,
            0,
            TIM_OCPolarity_High,
            TIM_OCNPolarity_High,
            0,
            0,
    };
    TIM_OC1Init(TIM14, (TIM_OCInitTypeDef*)&pwm);
    TIM_CCxCmd(TIM14, TIM_Channel_1, TIM_CCx_Enable);
}

static void initWdt(void) {
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	IWDG_SetReload(0x0FFF);
	IWDG_ReloadCounter();
	IWDG_Enable();
	DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP, ENABLE);
}

static void initADC(void) {
	const static ADC_InitTypeDef iface = {
		ADC_Resolution_12b,
		DISABLE,
		ADC_ExternalTrigConvEdge_None,
		0,
		ADC_DataAlign_Right,
		ADC_ScanDirection_Upward
	};

	ADC_DeInit(ADC1);
	ADC_Init(ADC1, (ADC_InitTypeDef*)&iface);
	ADC_ClockModeConfig(ADC1, ADC_ClockMode_SynClkDiv4);
	ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_239_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_7, ADC_SampleTime_239_5Cycles);

	initADC_NVIC();
	ADC_ContinuousModeCmd(ADC1, DISABLE);
	ADC_DiscModeCmd(ADC1, ENABLE);

    ADC_ITConfig(ADC1, ADC_IT_ADRDY | ADC_IT_EOC | ADC_IT_EOSEQ | ADC_IT_OVR | ADC_IT_AWD, ENABLE);

	ADC_GetCalibrationFactor(ADC1);
	ADC_Cmd(ADC1, ENABLE);

	s_adcTimerId = Timer_newArmed(ADC_TIMEOUT, true, onAdcTimeout, NULL);
}

static void initADC_NVIC(void) {
	static const NVIC_InitTypeDef nvic = {
			ADC1_IRQn,
			1,
			ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&nvic);
}

static void onAdcTimeout(uint32_t id, void *data) {
	Timer_disarm(id);
	ADC_StartOfConversion(ADC1);
}

void ADC1_IRQHandler(void) {

    static uint8_t channel;
    const int flag = ADC1->IER & ADC1->ISR;
    ADC1->ISR = flag;

    if (flag & ADC_IT_ADRDY) {
        ADC_StartOfConversion(ADC1);
    }

    if (flag & ADC_IT_EOC) {
        int val = adcToVoltsVbatt(ADC1->DR);
        val = adcRdiv(val, 270, 47);
        if (channel && val > 125)
            val -= 125;
        val |= channel++ << 24;
        EventQueue_Push(EVENT_ADC, (void*)val, NULL);
        if (!(flag & ADC_IT_EOSEQ))
            ADC_StartOfConversion(ADC1);//
    }
    if (flag & ADC_IT_EOSEQ) {
        Timer_rearm(s_adcTimerId);
        channel = 0;
    }
}

static inline int adcToVoltsVbatt(int adc) {
    return adc * 3300 / 4096;
}

static inline int adcRdiv(int adc, int rHi, int rLo) {
    return adc * (rHi + rLo) / rLo;
}
