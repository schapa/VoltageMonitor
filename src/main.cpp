
#include <stdio.h>
#include <stdlib.h>

#include "bsp.h"
#include "Queue.h"
#include "timers.h"
#include "systemTimer.h"

#define OVERRIDE_TOUT (BSP_TICKS_PER_SECOND * 60)
#define BLINK_TOUT (BSP_TICKS_PER_SECOND / 5)


static void onTimerPush(uint32_t id);

static uint32_t s_timId = INVALID_HANDLE;
static uint32_t s_timIdBlink = INVALID_HANDLE;
static void onOverrideTimeout(uint32_t id, void *data) {
}
static void onBlinkTimeout(uint32_t id, void *data) {
    static bool state;
    BSP_SetPinVal(BSP_Pin_LED_Red, state);
    BSP_SetPinVal(BSP_Pin_LED_Green, !state);
    state = !state;
}

int main(int argc, char* argv[]) {

	Timer_init(onTimerPush);
	BSP_Init();
    int vBatt = 0;
    int vBattPerc = 0;
    int vSolar = 0xFFFF;

	while (true) {
		Event_t event;
		EventQueue_Pend(&event);
		BSP_FeedWatchdog();
		uint32_t intVal = (uint32_t)event.data;
		switch (event.type) {
			case EVENT_SYSTICK:
				break;
			case EVENT_TIMCALL:
				Timer_onTimerCb(intVal);
				break;
			case EVENT_BUTTON:
			    if (s_timId == INVALID_HANDLE) {
			        s_timId = Timer_newArmed(OVERRIDE_TOUT, false, onOverrideTimeout, NULL);
			        s_timIdBlink = Timer_newArmed(BLINK_TOUT, true, onBlinkTimeout, NULL);
			    } else {
			        Timer_delete(s_timId);
                    Timer_delete(s_timIdBlink);
			        s_timId = INVALID_HANDLE;
			        s_timIdBlink = INVALID_HANDLE;
			    }
			    break;
			case EVENT_ADC: {
				if (!System_getUptime() && (System_getUptimeMs() < 300))
					break;
				const int volt = intVal & 0xFFFFFF;
                const int channel = intVal >> 24;
                static const int low = 9950;
                static const int full = 12450;
                if (!channel) {
                    vBatt = volt;
                    vBattPerc = 100 * (vBatt - low) / (full - low);
                    if (vBattPerc < 0)
                        vBattPerc = 0;
                    BSP_ShowCharge(vBattPerc);
                } else if (channel == 1) {
                    vSolar = volt;
                }
                if (s_timId != INVALID_HANDLE) {
                    if (vBatt > full) {
                        BSP_SetPinVal(BSP_Pin_SolarKey, true);
                        BSP_SetPinVal(BSP_Pin_BatteryKey, false);
                    } else {
                        BSP_SetPinVal(BSP_Pin_SolarKey, false);
                        BSP_SetPinVal(BSP_Pin_BatteryKey, false);
                    }
                    break;
                }
                if (vBatt > full) {
                    BSP_SetPinVal(BSP_Pin_SolarKey, true);
                    BSP_SetPinVal(BSP_Pin_BatteryKey, false);
                } else if (vBatt <= low && s_timId == INVALID_HANDLE) {
                    BSP_SetPinVal(BSP_Pin_SolarKey, false);
                    BSP_SetPinVal(BSP_Pin_BatteryKey, true);
                    BSP_SetPinVal(BSP_Pin_LED_Red, false);
                    BSP_SetPinVal(BSP_Pin_LED_Green, false);
                } else {
                    BSP_SetPinVal(BSP_Pin_SolarKey, false);
                    BSP_SetPinVal(BSP_Pin_BatteryKey, false);
                }

                BSP_SetPinVal(BSP_Pin_LED_Red, vSolar > vBatt);
                BSP_SetPinVal(BSP_Pin_LED_Green, vBattPerc > 50);
				break;
			}
			default:
				break;
		}
		EventQueue_Dispose(&event);
	}
	return 0;
}

static void onTimerPush(uint32_t id) {
	EventQueue_Push(EVENT_TIMCALL, (void*)id, NULL);
}
