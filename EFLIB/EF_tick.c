#include "EF_tick.h"

//Private variable
__IO u32 tickCounter=0;

//System tick initialization
void EF_TickInit(void)
{
	//1ms tick
	if(SysTick_Config(SystemCoreClock / 1000))
	{
		while(1);
	}
}

//Get system tick time (ms)
u32 EF_TickGetTimeMs(void)
{
	return (u32)tickCounter;
}

//delay n-ms function
void EF_TickDelayMs(u32 millisecond)
{
	u32 currentTime = EF_TickGetTimeMs();
	while (EF_TickGetTimeMs() - currentTime <= millisecond);
}

//SysTick interrupt function
void SysTick_Handler(void)
{
	tickCounter++;
}


