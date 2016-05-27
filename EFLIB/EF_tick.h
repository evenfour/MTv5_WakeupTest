#ifndef _TICK_H
#define _TICK_H

#include "stm32f4xx.h"
#define DelayMs(T) EF_TickDelayMs(T)

void 	EF_TickInit(void);
u32 	EF_TickGetTimeMs(void);
void 	EF_TickDelayMs(u32 nms);

#endif



