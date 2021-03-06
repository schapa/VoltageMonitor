
CFLAGS += \
	-I./inc \
	-I./src \
	-I./lib/ \
	
CFLAGS += \
	-fdata-sections \
	-ffunction-sections \
	\
	-DDEBUG \
	-DSTM32F030 \
	-DHSE_VALUE=8000000 \
	-DUSE_STDPERIPH_DRIVER \
	\
	-I./sdk/include/ \
	-I./sdk/include/arm \
	-I./sdk/include/cmsis/ \
	-I./sdk/include/cortexm/ \
	-I./sdk/include/diag \
	-I./sdk/include/stm32f0-stdperiph \
	
export SRC := \
	$(wildcard ./src/*.c) \
	$(wildcard ./src/*.cpp) \
	$(wildcard ./lib/src/*.c) \
	\
	./sdk/src/stm32f0-stdperiph/stm32f0xx_rcc.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_gpio.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_exti.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_syscfg.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_adc.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_iwdg.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_misc.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_tim.c \
	./sdk/src/stm32f0-stdperiph/stm32f0xx_dbgmcu.c \

