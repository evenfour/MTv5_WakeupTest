#ifndef ADS8330_H_
#define ADS8330_H_

#include "stm32f4xx.h"
#include "EF_tick.h"
#include "EF_spiflash.h"
#include "define.h"

#define MAN_2CH
#define ADS_SPI_BANDRATE 	SPI_BaudRatePrescaler_8
#define ADS_BUFF_MAX			128
#define ADS_BUFF_MAX_DUAL	256

#define ADS_CH_NUM			(u32)2
#define ADS_FREQ_L			(u32)20	//kHz
#define ADS_FREQ_FACTOR	(u32)2
#define ADS_FREQ_H			(u32)(ADS_FREQ_L*ADS_FREQ_FACTOR)	//kHz
#define ADS_CS_Pin   		GPIO_Pin_1  		//AD_CS  PA1
#define ADS_EOC_Pin  		GPIO_Pin_4  		//AD_EOC PC4
#define ADS_CONVST_Pin  GPIO_Pin_5 			//CONVST PC5

#define ADS_TEST_Port		GPIOC
#define ADS_TEST_Pin1		GPIO_Pin_10 //PC10
#define ADS_TEST_Pin2		GPIO_Pin_11	//PC11
#define ADS_TEST_Pin3   GPIO_Pin_12	//PC12

#define ADS_SCK_Pin			GPIO_Pin_5 // PA5
#define ADS_MISO_Pin 		GPIO_Pin_6 // PA6
#define ADS_MOSI_Pin 		GPIO_Pin_7 // PA7


#define ADS_CS_L GPIO_ResetBits(GPIOA,ADS_CS_Pin);__nop()
#define ADS_CS_H GPIO_SetBits(GPIOA,ADS_CS_Pin)

#define ADS_CONVST_L GPIO_ResetBits(GPIOC,ADS_CONVST_Pin);__nop()
#define ADS_CONVST_H GPIO_SetBits(GPIOC,ADS_CONVST_Pin)

#define TP1_L GPIO_ResetBits(ADS_TEST_Port,ADS_TEST_Pin1)
#define TP1_H GPIO_SetBits(ADS_TEST_Port,ADS_TEST_Pin1)

#define TP2_L GPIO_ResetBits(ADS_TEST_Port,ADS_TEST_Pin2)
#define TP2_H GPIO_SetBits(ADS_TEST_Port,ADS_TEST_Pin2)

#define TP3_L GPIO_ResetBits(ADS_TEST_Port,ADS_TEST_Pin3)
#define TP3_H GPIO_SetBits(ADS_TEST_Port,ADS_TEST_Pin3)

void ADS_Init(void);
void ADS_Start(void);
void ADS_Stop(void);

#endif



