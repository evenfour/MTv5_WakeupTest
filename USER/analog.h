#ifndef ANALOG_H_
#define ANALOG_H_

#include "stm32f4xx.h"
#include "EF_spiflash.h"

#define ANALOG_FREQ 					20				//kHz
#define ANALOG_GPIO_Port 			GPIOB
#define ANALOG_GPIO_CH0_Pin		GPIO_Pin_0
#define ANALOG_GPIO_RCC				RCC_AHB1Periph_GPIOB
#define ANALOG_ADC_RCC				RCC_APB2Periph_ADC1
#define ANALOG_TIM_RCC				RCC_APB1Periph_TIM3

#define ANALOG_BUFFSIZE 			64
#define ANALOG_BUFFSIZE_DUAL	128

#define ANALOG_WRITEMEM_ON

typedef union
{
	u16 HalfWord[ANALOG_BUFFSIZE];
	u8	Byte[ANALOG_BUFFSIZE_DUAL];
}Datalog_Structure;

void analog_Init(void);
void ADC_Start(void);
void ADC_Stop(void);

#endif


