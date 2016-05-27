#include "magnet.h"

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;

void MAGNET_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	//MAGNET_CS_H; // Chip Select Disabled

	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 559;
	TIM_TimeBaseStructure.TIM_Prescaler = 999;
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM4,DISABLE);
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

u32 tim4_counter=0;
extern u16 magnet_data[3][1800];

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET)
	{		
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
		TP2_L;
		__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();
		//magnet_data[0][tim4_counter]=Magnet_ReadDataX();
		//magnet_data[1][tim4_counter]=Magnet_ReadDataY();
		//magnet_data[2][tim4_counter]=Magnet_ReadDataZ();
		//Magnet_Start();
		//tim4_counter++;
		TP2_H;
	}
}
	
 u16 datax;
 u16 datay;
 u16 dataz;
 u32 mag_count;
 extern u8 record_Flag;

/*
void EXTI2_IRQHandler()
{
	u32 count=mag_count%100;
	mag_data[0][count]=Magnet_ReadDataX();
	mag_data[1][count]=Magnet_ReadDataY();
	mag_data[2][count]=Magnet_ReadDataZ();
	mag_count++;
	EXTI_ClearITPendingBit(EXTI_Line2);
}
*/

u8 MAGNET_SPI_RW_Byte(SPI_TypeDef* SPIx,u8 Byte)
{
	while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_I2S_SendData(SPIx, Byte);
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
		;
	return SPI_I2S_ReceiveData(SPIx);
}

u8 MAGNET_Write_Reg(u8 reg, u8 value)
{
	u8 status;
	MAGNET_CS_L;
	status =MAGNET_SPI_RW_Byte(SPI2,reg); //Send address first
	MAGNET_SPI_RW_Byte(SPI2,value); //Send Value
	MAGNET_CS_H;
	return(status);
}

u8 MAGNET_Read_Reg(u8 reg)
{
	u8 reg_val;
	MAGNET_CS_L;
	MAGNET_SPI_RW_Byte(SPI2,reg|0x80); //Send address first
  reg_val=MAGNET_SPI_RW_Byte(SPI2,0XFF); //Read value
  MAGNET_CS_H;
  return(reg_val);
}

u8 Magnet_VerifyID(void)
{
	u8 reg;
	reg=MAGNET_Read_Reg(MAGNET_CHIPID);
	if(reg==0x32)
		return 1;
	else
		return 0;
}

void Magnet_Config(void)
{
	MAGNET_Write_Reg(MAGNET_POWERMODE,0xFF);
	DelayMs(10);
	MAGNET_Write_Reg(MAGNET_POWERMODE,0x01);
	DelayMs(10);
	MAGNET_Write_Reg(MAGNET_REPXY,0x00);
	DelayMs(10);
	MAGNET_Write_Reg(MAGNET_REPZ,0x00);
	DelayMs(10);
	MAGNET_Write_Reg(MAGNET_INTSET,0x80);
	DelayMs(10);
}

void Magnet_Start(void)
{
	u8 reg=MAGNET_Read_Reg(MAGNET_OPMODE);
	MAGNET_Write_Reg(MAGNET_OPMODE,reg&0xFB);
}

u16 Magnet_ReadDataX(void)
{
	u8 lsb,msb;
	u16 magx;
	lsb=MAGNET_Read_Reg(MAGNET_DATAX_LSB);
	msb=MAGNET_Read_Reg(MAGNET_DATAX_MSB);
	magx=((u16)lsb+(u16)msb*256)>>3;
	if(magx<=4095)		
		magx=magx+4096;
	else
		magx=magx-4096;
	return magx;
}

u16 Magnet_ReadDataY(void)
{
	u8 lsb,msb;
	u16 magy;
	lsb=MAGNET_Read_Reg(MAGNET_DATAY_LSB);
	msb=MAGNET_Read_Reg(MAGNET_DATAY_MSB);
	magy=((u16)lsb+(u16)msb*256)>>3;
	if(magy<=4095) 
		magy=magy+4096;
	else 
		magy=magy-4096;
	return magy;
}

u16 Magnet_ReadDataZ(void)
{
	u8 lsb,msb;
	u16 magz;
	lsb=MAGNET_Read_Reg(MAGNET_DATAZ_LSB);
	msb=MAGNET_Read_Reg(MAGNET_DATAZ_MSB);
	magz=((u16)lsb+(u16)msb*256)>>1;
	if(magz<=16383)		
		magz=magz+16384;
	else
		magz=magz-16384;
	return magz;
}
