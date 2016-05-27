#ifndef _LED_H
#define _LED_H

#include "stm32f4xx.h"
#include "EF_tick.h"

#define LED_RCC				RCC_AHB1Periph_GPIOC				
#define LED_PORT  		GPIOC
#define LED_Red				GPIO_Pin_2	//PC2
#define LED_Green			GPIO_Pin_1	//PC1
#define LED_Blue			GPIO_Pin_0	//PC0
#define LED_All				(LED_Red|LED_Green|LED_Blue)

void led_Init(void);
void led_On(u16 color);
void led_Off(u16 color);
void led_Toggle(u16 color);
void led_Flash(u16 color,u8 times);
static void led_ClockInit(void);
static void led_GPIOInit(void);	

#endif

