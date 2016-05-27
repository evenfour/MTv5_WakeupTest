#include "nrf2401.h"

//#define CODE_M1
#define CODE_M2

u8 NRF_M0_ADDRESS[NRF_ADR_WIDTH]={0x01,0x0A,0x0A};
u8 NRF_M1_ADDRESS[NRF_ADR_WIDTH]={0x11,0x0A,0x0A};
u8 NRF_M2_ADDRESS[NRF_ADR_WIDTH]={0x12,0x0A,0x0A};

#define RFID_CH 0x02
void NRF24L01_ExitInit(void);
	
void NRF24L01_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//RCC configuration
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //GPIOB CLOCK
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //GPIOC CLOCK

	//CE
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CE_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//CSN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	NRF24L01_CE_L; // RF PWR OFF
	NRF24L01_CSN_H; // Chip Select Disabled
	//NRF24L01_ExitInit();
	SPI2_Init();
}

void NRF24L01_ExitInit(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //PC8
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC,EXTI_PinSource8);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	EXTI_ClearITPendingBit(EXTI_Line8);	
	EXTI_InitStructure.EXTI_Line=EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel=EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

u8 nRFreceivedCmd=0;

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8)==SET)
	{
		nRFreceivedCmd = 1;
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //GPIOB CLOCK
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); //SPI2 CLOCK
	//SPI SCK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//SPI MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//SPI MISO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//SPI configuration
	SPI_DeInit(SPI2);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //Full Duplex
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master; //As master
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; //8bit
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; 
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; //MSB first
  SPI_InitStructure.SPI_CRCPolynomial = 7; //CRC
  SPI_Init(SPI2, &SPI_InitStructure);
	SPI_ClearITPendingBit(SPI2,SPI_IT_TXE|SPI_IT_RXNE);
  SPI_Cmd(SPI2,ENABLE);
}

// SPI basic Read/Write function
u8 SPI_RW_Byte(SPI_TypeDef* SPIx,unsigned char Byte)
{
		while( SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)   	  __nop();
		SPI_I2S_SendData(SPIx, Byte);
		while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)  	  __nop();
		return SPI_I2S_ReceiveData(SPIx);
}

//Flush TX
void NRF24L01_Flush_TX(void)
{
	NRF24L01_CSN_L;
	__nop();__nop();__nop();
	SPI_RW_Byte(SPI2,NRF_FLUSH_TX);
	__nop();__nop();__nop();
	NRF24L01_CSN_H;	
}

//Flush RX
void NRF24L01_Flush_RX(void)
{
	NRF24L01_CSN_L;
	__nop();__nop();__nop();
	SPI_RW_Byte(SPI2,NRF_FLUSH_RX);
	__nop();__nop();__nop();
	NRF24L01_CSN_H;	
}

//Write nRF register
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
	NRF24L01_CSN_L;
	__nop();__nop();__nop();
	status =SPI_RW_Byte(SPI2,NRF_WRITE_REG+reg); 	//Send address first
	SPI_RW_Byte(SPI2,value); 						//Send Value
	__nop();__nop();__nop();
	NRF24L01_CSN_H;
	return(status);
}

//Read nRF register
u8 NRF24L01_Read_Reg(u8 reg)//Read register
{
	u8 reg_val;	    
	NRF24L01_CSN_L;
	__nop();__nop();__nop();
	SPI_RW_Byte(SPI2,NRF_READ_REG+reg); 	//Send address first
  	reg_val=SPI_RW_Byte(SPI2,0XFF); 		//Read value
	__nop();__nop();__nop();
  NRF24L01_CSN_H;
  return(reg_val);						//Return the register value
}	

//Read buff
u8 NRF24L01_Read_Buf(u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;
  NRF24L01_CSN_L;
	__nop();__nop();__nop();
  status=SPI_RW_Byte(SPI2,NRF_RD_RX_PLOAD); 	//Send address first
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)
 	{
	  pBuf[u8_ctr]=SPI_RW_Byte(SPI2,0XFF); 		//Read
  }
	__nop();__nop();__nop();
	NRF24L01_CSN_H;
  return status;
}

//Write buff
u8 NRF24L01_Write_Buf(u8 *pBuf, u8 len)
{
	u8 status,u8_ctr;	    
 	NRF24L01_CSN_L;
	__nop();__nop();__nop();
  status = SPI_RW_Byte(SPI2,NRF_WR_TX_PLOAD); 	//Send address first
  for(u8_ctr=0; u8_ctr<len; u8_ctr++)
  {
  	SPI_RW_Byte(SPI2,*pBuf++); 					//Write
  }
	__nop();__nop();__nop();
  NRF24L01_CSN_H;
  return status;
}

//Read nRF address
u8 NRF24L01_Read_Addr(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;
  	NRF24L01_CSN_L;
  	status=SPI_RW_Byte(SPI2,NRF_READ_REG+reg); 	//Send address first
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)
 	{
	  pBuf[u8_ctr]=SPI_RW_Byte(SPI2,0XFF); 		//Read
  	}
	NRF24L01_CSN_H;
  	return status;
}

//Write nRF address
u8 NRF24L01_Write_Addr(u8 reg,u8 *pBuf,u8 len)
{
	u8 status,u8_ctr;
  	NRF24L01_CSN_L;
  	status=SPI_RW_Byte(SPI2,NRF_WRITE_REG+reg); //Send address first
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++)
	{
 		SPI_RW_Byte(SPI2,*pBuf++); 				//Write
  	}
	NRF24L01_CSN_H;
  	return status;
}

// Send data packet
u8 NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta=0x00;
	NRF24L01_CE_L;
  	NRF24L01_Write_Buf(txbuf,NRF_PLOAD_WIDTH);
 	NRF24L01_CE_H;   

 	do
 	{
 		sta= (NRF24L01_Read_Reg(NRF_STATUS)) & (NRF_MAX_TX | NRF_TX_OK);
 	} while(sta==0x00);

	NRF24L01_Write_Reg(NRF_STATUS,sta);
	
	if(sta & NRF_MAX_TX)
	{
		NRF24L01_Flush_TX();
		return NRF_MAX_TX;
	}
	if(sta & NRF_TX_OK)
	{
		NRF24L01_Flush_TX();
		return NRF_TX_OK;
	}
	return 0x00;
}

// Receive data packet
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;		    							   
	sta=NRF24L01_Read_Reg(NRF_STATUS);
	NRF24L01_Write_Reg(NRF_STATUS,sta);
	if(sta & NRF_RX_OK )
	{
		NRF24L01_Read_Buf(rxbuf,NRF_PLOAD_WIDTH);
		NRF24L01_Flush_RX();
		return NRF_RX_OK;
	}	   
	return 0x00;
}

u8 tmpreg=0xff;

void RX_Mode(void)
{
		NRF24L01_CE_L;
    NRF24L01_Write_Addr(NRF_TX_ADDR,NRF_M0_ADDRESS,NRF_ADR_WIDTH);
#ifdef CODE_M1
    NRF24L01_Write_Addr(NRF_RX_ADDR_P0,NRF_M1_ADDRESS,NRF_ADR_WIDTH);
#endif
#ifdef CODE_M2
    NRF24L01_Write_Addr(NRF_RX_ADDR_P0,NRF_M2_ADDRESS,NRF_ADR_WIDTH);
#endif
	  NRF24L01_Write_Reg(NRF_EN_AA,0x01);  //ACK
    NRF24L01_Write_Reg(NRF_EN_RXADDR,0x01);
	
  	NRF24L01_Write_Reg(NRF_SETUP_AW,0x01);
		NRF24L01_Write_Reg(NRF_SETUP_RETR,0x1A);
  	NRF24L01_Write_Reg(NRF_RF_CH,RFID_CH); //RF channel
		NRF24L01_Write_Reg(NRF_RF_SETUP,0x26); //RF setup
	
		NRF24L01_Write_Reg(NRF_RX_PW_P0,NRF_PLOAD_WIDTH);
    NRF24L01_Write_Reg(NRF_CONFIG,0x0F);//PWR_UP,EN_CRC,16BIT_CRC,
		NRF24L01_CE_H;
}						 

u8 TX_Mode(u8 *txbuf)//TX Configuration
{
	u8 sta=0xff;
	NRF24L01_CE_L;
  NRF24L01_Write_Addr(NRF_TX_ADDR,NRF_M0_ADDRESS,NRF_ADR_WIDTH);
#ifdef CODE_M1
   NRF24L01_Write_Addr(NRF_RX_ADDR_P0,NRF_M1_ADDRESS,NRF_ADR_WIDTH);
#endif
#ifdef CODE_M2
   NRF24L01_Write_Addr(NRF_RX_ADDR_P0,NRF_M2_ADDRESS,NRF_ADR_WIDTH);
#endif
  NRF24L01_Write_Buf(txbuf,NRF_PLOAD_WIDTH);
  NRF24L01_Write_Reg(NRF_EN_AA,0x01);  //ACK
  NRF24L01_Write_Reg(NRF_EN_RXADDR,0x01); //
  NRF24L01_Write_Reg(NRF_SETUP_AW,0x01);
  NRF24L01_Write_Reg(NRF_SETUP_RETR,0x1a); //Auto resend
  NRF24L01_Write_Reg(NRF_RF_CH,RFID_CH); //RF channel
	NRF24L01_Write_Reg(NRF_RF_SETUP,0x26); //RF setup
	NRF24L01_Write_Reg(NRF_CONFIG,0x0e); //Tans. to TX mode
	NRF24L01_CE_H;
	DelayMs(1);
	NRF24L01_CE_L;

 	do{
 		sta= (NRF24L01_Read_Reg(NRF_STATUS)) & (NRF_MAX_TX | NRF_TX_OK);
 	}while(sta==0x00);
	NRF24L01_Write_Reg(NRF_STATUS,sta);

	if(sta & NRF_MAX_TX)
	{
		return NRF_MAX_TX;
	}
	if(sta & NRF_TX_OK)
	{
		return NRF_TX_OK;
	}
	return 0x00;

}		  




