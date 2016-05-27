#include "ads8330.h"

u16 ADS_SPI_RW(u16 value);
u16 tmp;

void ADS_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	TIM_TimeBaseInitTypeDef TimBaseInitStructure;

	//RCC configuration
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //GPIOA CLOCK
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //GPIOB CLOCK
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //GPIOC CLOCK
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //SPI1
	//Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	//TIMER

	//CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = ADS_CS_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//INT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = ADS_EOC_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//CONVST
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = ADS_CONVST_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	//For test
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = ADS_TEST_Pin1 | ADS_TEST_Pin2 | ADS_TEST_Pin3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_Init(ADS_TEST_Port, &GPIO_InitStructure);

	ADS_CS_H;
	ADS_CONVST_H;
	
	TP1_H;
	TP2_H;
	TP3_H;
	
	//SPI SCK
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Pin = ADS_SCK_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//SPI MISO
	GPIO_InitStructure.GPIO_Pin = ADS_MISO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	//SPI MOSI
	
	GPIO_InitStructure.GPIO_Pin = ADS_MOSI_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//SPI configuration
	SPI_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //Full Duplex
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //As master
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //8bit
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 
	SPI_InitStructure.SPI_BaudRatePrescaler = ADS_SPI_BANDRATE; /////////////////////////BAUDRATE
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //MSB first
	SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_ClearITPendingBit(SPI2,SPI_IT_TXE|SPI_IT_RXNE);
	SPI_Cmd(SPI1,ENABLE);
		
	//Timer
	TimBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimBaseInitStructure.TIM_Period = (105-1);
	TimBaseInitStructure.TIM_Prescaler = (1600/(ADS_FREQ_H*ADS_CH_NUM) - 1); ///set sample freq.
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);
	TIM_TimeBaseInit(TIM3,&TimBaseInitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource4);
	
	EXTI_ClearITPendingBit(EXTI_Line4);	
	
	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel=EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd=DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	///////////////////////////////////////////
	ADS_SPI_RW(0xF000);
	DelayMs(10);
	ADS_SPI_RW(0xEEFF); //0xEEBF 0xE7FF 0xE6FD-1110,0110,1111,1101 auto
	tmp = ADS_SPI_RW(0xC000);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	NVIC_EnableIRQ(EXTI4_IRQn);
	EXTI_ClearITPendingBit(EXTI_Line4);
}

// SPI basic Read/Write function
u16 ADS_SPI_RW_Byte(SPI_TypeDef* SPIx,u16 data)
{
	while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)   	  __nop();
	SPI_I2S_SendData(SPIx, data);
	while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)  	  __nop();
	return SPI_I2S_ReceiveData(SPIx);
}

u16 ADS_SPI_RW(u16 data)
{
	u16 tmp;
	ADS_CS_L;
	tmp = ADS_SPI_RW_Byte(SPI1,(u8)(data>>8));
	tmp = tmp<<8;
	tmp = tmp+ADS_SPI_RW_Byte(SPI1,data&(0xff));
	ADS_CS_H;
	return tmp;
}

u16 ADS_SPI_RW_TAG(u16 data, u8* ch)
{
	u16 tmp;
	ADS_CS_L;
	tmp = ADS_SPI_RW_Byte(SPI1,(u8)(data>>8));
	tmp = tmp << 8;
	tmp = tmp + ADS_SPI_RW_Byte(SPI1,data&(0xff));
	*ch = ADS_SPI_RW_Byte(SPI1,0xff)>>7;
	ADS_CS_H;
	return tmp;
}

u16 data_page0[ADS_BUFF_MAX];
u16 data_page1[ADS_BUFF_MAX];

__IO u16 data_index = 0;
__IO u32 data_count = 0;

double voltage_ch0=0;
double voltage_ch1=0;

__IO u8 selectChannel=0;
__IO u8 selectPage=0;

u8 writeFlag = 0;

extern u32 data_addr;

double converVoltage(u16 *data)
{
	
	double average;
	u32 i;
	average=0.0;
	for(i=0;i<ADS_BUFF_MAX;i++)
	{
		average += (double)data[i];
	}
	average = average/(double)(ADS_BUFF_MAX);
	return average*3300.0/65536.0;
}

u8 channel=0;
u8 ad_startFlag;

void EXTI4_IRQHandler(void)
{
	ADS_CONVST_H;
	if(selectPage == 0)
	{
			data_page0[data_index++]=ADS_SPI_RW_TAG(0xDFFF,&channel);
			if( 	ad_startFlag == 1 )
			{
				if(((channel == 0) && (data_index%2==0)) || ((channel == 1) && (data_index%2==1)) )
					data_index=data_index-1;
				ad_startFlag = 0;
			}
	}
	else
	{
			data_page1[data_index++]=ADS_SPI_RW_TAG(0xDFFF,&channel);
	}
	data_count++;

	TP1_H;
	if( data_index == ADS_BUFF_MAX )
	{
		data_index = 0;
		if(selectPage==0)
		{
			selectPage = 1;
		}
		else
		{
			selectPage = 0;
		}
		writeFlag = 1;
	}
	EXTI_ClearITPendingBit(EXTI_Line4);
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{	
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	
		ADS_CONVST_L;TP1_L;
	}
}

void ADS_Start(void)
{
	data_index = 0;
	data_count = 0;
	ad_startFlag = 1;	
	ADS_SPI_RW(0xEEFF);
	NVIC_EnableIRQ(EXTI4_IRQn);
	EXTI_ClearITPendingBit(EXTI_Line4);
	TIM_Cmd(TIM3,ENABLE);
	TIM_Cmd(TIM4,ENABLE);
}

void ADS_Stop(void)
{
	TP1_H;
	TP2_H;
	TP3_H;
	TIM_Cmd(TIM3,DISABLE);	
	TIM_Cmd(TIM4,DISABLE);
	NVIC_DisableIRQ(EXTI4_IRQn);
	EXTI_ClearITPendingBit(EXTI_Line4);
}








