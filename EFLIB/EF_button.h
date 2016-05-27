#ifndef _EF_BUTTON_H
#define _EF_BUTTON_H

#include "stm32f4xx.h"
#include "EF_tick.h"
#include "define.h"

#define BUTTON_PORT GPIOA
#define BUTTON_PIN	GPIO_Pin_0
#define BUTTON_RCC	RCC_AHB1Periph_GPIOA

#define PB_PORT GPIOB
#define PB_RCC	RCC_AHB1Periph_GPIOB
#define PB1_PIN	GPIO_Pin_1
#define PB2_PIN	GPIO_Pin_2

void EF_buttonInit(void);
	
#endif


