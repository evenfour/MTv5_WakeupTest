#include "led.h"

void led_Init()
{
	led_ClockInit();
	led_GPIOInit();
	led_Off(LED_All);
}

void led_ClockInit(void)
{
	RCC_AHB1PeriphClockCmd(LED_RCC,ENABLE);
}

void led_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = 	LED_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;
	GPIO_Init(LED_PORT,&GPIO_InitStructure);
}

void led_On(u16 color)
{
	GPIO_SetBits(LED_PORT,color);
}

void led_Off(u16 color)
{
	GPIO_ResetBits(LED_PORT,color);
}

void led_Toggle(u16 color)
{
	LED_PORT->ODR ^= color;
}

void led_Flash(u16 color,u8 times)
{
	u8 i;
	led_Off(color);
	for(i=0;i<times;i++)
	{
		led_On(color);
		DelayMs(80);
		led_Off(color);
		DelayMs(80);
	}
}








