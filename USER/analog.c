#include "analog.h"
#include "led.h"

//#define TIM3_INT_ON
//#define ANALOG_INT_ON
#define ANALOG_DMA_ON

static TIM_TimeBaseInitTypeDef TimBaseInitStructure;
static NVIC_InitTypeDef NVIC_InitStructure;
static GPIO_InitTypeDef GPIO_InitStructure;
static ADC_CommonInitTypeDef ADC_CommonInitStructure;
static ADC_InitTypeDef ADC_InitStructure;

#ifdef ANALOG_DMA_ON
static DMA_InitTypeDef DMA_InitStructure;
#endif

#ifdef ANALOG_DMA_ON
static Datalog_Structure analog_DataCache;
//static Datalog_Structure temp;
#endif

void ADC_Start(void)
{
	TIM_Cmd(TIM3,ENABLE);
}

void ADC_Stop(void)
{
	TIM_Cmd(TIM3,DISABLE);
}

u16 period;
u16 prescaler;

void analog_Init(void)
{
	
	period=(u16)(168/4)-1;
	prescaler=(u16)(2000/ANALOG_FREQ)-1;
	
	//Clock
	RCC_APB1PeriphClockCmd(ANALOG_TIM_RCC,ENABLE);
	RCC_AHB1PeriphClockCmd(ANALOG_GPIO_RCC,ENABLE);
	RCC_APB2PeriphClockCmd(ANALOG_ADC_RCC,ENABLE);
	
	//GPIO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Pin = ANALOG_GPIO_CH0_Pin;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ANALOG_GPIO_Port, &GPIO_InitStructure);

	//ADC Common
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInit(&ADC_CommonInitStructure);

	//ADC
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Falling;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_8,1,ADC_SampleTime_28Cycles);
	
#ifdef ANALOG_INT_ON	
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
#endif
	
	//Timer
	TimBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimBaseInitStructure.TIM_Period = period;
	TimBaseInitStructure.TIM_Prescaler = prescaler;
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);
	TIM_TimeBaseInit(TIM3,&TimBaseInitStructure);

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

#ifdef ANALOG_INT_ON
	//NVIC
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
	
#ifdef TIM3_INT_ON 
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#ifdef ANALOG_DMA_ON
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	DMA_Cmd(DMA2_Stream0,DISABLE);
	while (DMA_GetCmdStatus(DMA2_Stream0) != DISABLE)		;
	DMA_DeInit(DMA2_Stream0);
	DMA_InitStructure.DMA_BufferSize=ANALOG_BUFFSIZE;
	DMA_InitStructure.DMA_Channel=DMA_Channel_0;
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_FIFOMode=DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold=DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_Memory0BaseAddr=(u32)&analog_DataCache.HalfWord[0];
	DMA_InitStructure.DMA_MemoryBurst=DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode=DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr=(u32)&(ADC1->DR);
	DMA_InitStructure.DMA_PeripheralBurst=DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority=DMA_Priority_High;
	
	DMA_Init(DMA2_Stream0,&DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0,ENABLE);
	DMA_ClearITPendingBit(DMA1_Stream0,DMA_IT_TCIF0);
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStructure);
#endif

	TIM_Cmd(TIM3,DISABLE);
	
#ifdef ANALOG_DMA_ON
	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE); 
	ADC_DMACmd(ADC1, ENABLE);
#endif

	ADC_Cmd(ADC1, ENABLE);
}

#ifdef ANALOG_INT_ON
extern u32 flash_DataCacheAddress;
extern u32 flash_DataCacheSize;
static u16 tmp_result;
static u8 tmp_result_h,tmp_result_l;
void ADC_IRQHandler(void)
{
	if(ADC_GetITStatus(ADC1,ADC_IT_EOC)==SET)
	{
		
		tmp_result=ADC_GetConversionValue(ADC1);
		tmp_result_h= (tmp_result & 0xFF00) >>8;
		tmp_result_l= tmp_result & 0xFF;
		EF_spiFlashPageWrite(flash_DataCacheAddress,&tmp_result_h,1);
		EF_spiFlashPageWrite(flash_DataCacheAddress+1,&tmp_result_l,1);
		flash_DataCacheAddress += 2;
		flash_DataCacheSize +=1;
		ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
	}
}
#endif

#ifdef TIM3_INT_ON 
void TIM3_IRQHandler(void)
{
 if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
 {
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
 }
}
#endif

#ifdef ANALOG_DMA_ON
extern u32 flash_DataCacheAddress;
extern u32 flash_DataCacheSize;
void DMA2_Stream0_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0)==SET)
	{
//#ifdef ANALOG_WRITEMEM_ON		
		EF_spiFlashWrite(flash_DataCacheAddress,&analog_DataCache.Byte[0],ANALOG_BUFFSIZE_DUAL);
		//EF_spiFlashRead(flash_DataCacheAddress,&temp.Byte[0],ANALOG_BUFFSIZE_DUAL);
//#endif
		flash_DataCacheSize += ANALOG_BUFFSIZE;
		flash_DataCacheAddress += ANALOG_BUFFSIZE_DUAL;
		DMA_ClearITPendingBit(DMA2_Stream0,DMA_IT_TCIF0);
	}
}
#endif



