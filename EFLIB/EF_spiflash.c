#include "EF_spiFlash.h"
#include "ads8330.h"

/***Private functions***/
void EF_spiFlashGpioInit(void);
void EF_spiFlashSpiInit(void);
void EF_spiFlash4BModeOn(void);
void EF_spiFlash4BModeOff(void);
u8 EF_spiFlashGetStatusReg(void);
u8 EF_spiFlashRW(u8 data);
void EF_spiFlashDmaInit(void);
void EF_spiFlashNvicInit(void);
void EF_spiFlashDmaTx(u8* buff,u16 length);
DMA_InitTypeDef DMA_Tx_InitStructure;
u8 DMA_Tx_Busy = 0;

/***Public Functions***/
//spi flashrom initialization
void EF_spiFlashInit(void)
{
	u8 flash_init_retry=5;
	EF_spiFlashGpioInit();
	EF_spiFlashSpiInit();
	EF_spiFlashDmaInit();
	EF_spiFlashNvicInit();
	__nop();
	while(EF_spiFlashGetID()!=(u32)MX66_DeviceID && flash_init_retry>0)
	{
		EF_spiFlashSoftReset();
		DelayMs(320);
		flash_init_retry--;
	}
	if(flash_init_retry == 0)
	{
		////////////////////////////
		led_On(LED_Red);
		while(1)
			__nop();
	}
	DelayMs(10);
	EF_spiFlash4BModeOn();
	DelayMs(10);
}

void EF_spiFlashDmaInit(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	DMA_DeInit(DMA1_Stream4);

	DMA_Tx_InitStructure.DMA_Channel = DMA_Channel_0;	
	DMA_Tx_InitStructure.DMA_PeripheralBaseAddr = (u32)&(SPI2->DR);
	DMA_Tx_InitStructure.DMA_Memory0BaseAddr = 0x00;
	DMA_Tx_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_Tx_InitStructure.DMA_BufferSize = MX66_Page_Size;
	DMA_Tx_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Tx_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Tx_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_Tx_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_Tx_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_Tx_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Tx_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_Tx_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_Tx_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_Tx_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream4,&DMA_Tx_InitStructure);
	DMA_ITConfig(DMA1_Stream4,DMA_IT_TC|DMA_IT_TE,ENABLE);
	DMA_Cmd(DMA1_Stream4,DISABLE);
		
}

void EF_spiFlashNvicInit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//spi flashrom gpio initialization
void EF_spiFlashGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/***Clock***/
	RCC_AHB1PeriphClockCmd( SPIFLASH_SPI_GPIO_RCC | SPIFLASH_CS_GPIO_RCC ,ENABLE);
	/***SCK,MISO,MOSI***/
	GPIO_InitStructure.GPIO_Pin = SPIFLASH_SPI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_PinAFConfig(SPIFLASH_SPI_PORT, SPIFLASH_SPI_SCK_PINSOURCE,  SPIFLASH_SPI_AF);	//SCK AF
	GPIO_PinAFConfig(SPIFLASH_SPI_PORT, SPIFLASH_SPI_MISO_PINSOURCE, SPIFLASH_SPI_AF);	//MISO AF
	GPIO_PinAFConfig(SPIFLASH_SPI_PORT, SPIFLASH_SPI_MOSI_PINSOURCE, SPIFLASH_SPI_AF);	//MOSI AF	
	GPIO_Init(SPIFLASH_SPI_PORT, &GPIO_InitStructure);
	/***CS***/
	GPIO_InitStructure.GPIO_Pin = SPIFLASH_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(SPIFLASH_CS_PORT, &GPIO_InitStructure);
	SPIFLASH_CS_H;		
	
	//Disable other device on EVM
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE,GPIO_Pin_3);	
}

//spi initialization
void EF_spiFlashSpiInit(void)
{
	SPI_InitTypeDef SPI_InitStruct;
	/***Clock***/
	#ifdef SPIFLASH_USE_SPI1
		RCC_APB2PeriphClockCmd(SPIFLASH_SPI_RCC, ENABLE);
	#else
		RCC_APB1PeriphClockCmd(SPIFLASH_SPI_RCC, ENABLE);
	#endif
	/***SPI***/
	SPI_Cmd(SPIFLASH_SPI,DISABLE);
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;//SPI_CPOL_Low;				// CPOL = 0 --> clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;	// CPHA = 0 --> data is sampled at the first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = MX66_Baudrate; // APB2@42MHz and APB1@21MHz
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial=7;
	SPI_ClearITPendingBit(SPIFLASH_SPI,SPI_IT_TXE|SPI_IT_RXNE);
	
	SPI_Init(SPIFLASH_SPI,&SPI_InitStruct);	
	SPI_Cmd(SPIFLASH_SPI,ENABLE);
	EF_spiFlashRW(0xFF);
	EF_spiFlashRW(0xFF);
}

//spi basic read/write function
u8 EF_spiFlashRW(u8 data)
{
	while( DMA_Tx_Busy == 1) __nop();
	while( SPI_GetFlagStatus(SPIFLASH_SPI,SPI_I2S_FLAG_TXE)==RESET ) 		__nop();
	SPI_SendData(SPIFLASH_SPI,(u16)data);
	while( SPI_GetFlagStatus(SPIFLASH_SPI,SPI_I2S_FLAG_RXNE)==RESET ) 	__nop();
	return (u8)SPI_ReceiveData(SPIFLASH_SPI);
}

//flashrom ID verification
u32 EF_spiFlashGetID(void)
{
	__IO u8 ID[3]={0x00,0x00,0x00};
	u32 flashID=0;
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_RDID);
	ID[0]= EF_spiFlashRW(SPIFLASH_CMD_DUMMY);
	ID[1]= EF_spiFlashRW(SPIFLASH_CMD_DUMMY);
	ID[2]= EF_spiFlashRW(SPIFLASH_CMD_DUMMY);
	SPIFLASH_CS_H; 
	flashID = ((u32)ID[0]<<16)+((u32)ID[1]<<8)+((u32)ID[2]);
	__nop();
	return flashID;
}

//enable 4-Byte mode (32bit address)
void EF_spiFlash4BModeOn(void)	
{
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_EN4B);
	SPIFLASH_CS_H;
}
//disable 4-Byte mode (32bit address)
void EF_spiFlash4BModeOff(void)
{
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_EX4B);
	SPIFLASH_CS_H;
}

//Get status register
u8 EF_spiFlashGetStatusReg(void)
{
	u8 statusReg=0;
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_RDSR);
	statusReg = EF_spiFlashRW(SPIFLASH_CMD_DUMMY);
	SPIFLASH_CS_H;
	return statusReg;
}

//Enable write flash
void EF_spiFlashWriteEnable(void)
{
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_WREN);
	SPIFLASH_CS_H;
}
//Disable write flash
void EF_spiFlashWriteDisable(void)
{
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_WRDI);
	SPIFLASH_CS_H;	
}

//Is busy
u8 EF_spiFlashIsBusy(void)
{
	return ((EF_spiFlashGetStatusReg() & MEM_WIP_MASK) == MEM_WIP_MASK);
}

//Is write enable
u8 EF_spiFlashIsWriteEnabled(void)
{
	return ((EF_spiFlashGetStatusReg() & MEM_WEL_MASK) == MEM_WEL_MASK)	;
}

//wait for flash ready
EF_spiFlashStatus EF_spiFlashWaitForReady(u32 waitTime)
{
	u32 startTime = EF_TickGetTimeMs();
	while(EF_spiFlashIsBusy())
	{
		if ( (EF_TickGetTimeMs()-startTime) > waitTime )
			return SPIFLASH_STATUS_TIMEOUT;
	}
	return SPIFLASH_STATUS_NOERR;
}

EF_spiFlashStatus EF_spiFlashEraseAll(void)
{
	while( EF_spiFlashIsBusy())	__nop();
	if (!EF_spiFlashIsWriteEnabled())	
		EF_spiFlashWriteEnable();
	SPIFLASH_CS_L;
	//Request chip erase all
	EF_spiFlashRW(SPIFLASH_CMD_CE);
	SPIFLASH_CS_H;
	//wait till memory finish
	return EF_spiFlashWaitForReady(MX66_ChipEraseCycleTime);
}

EF_spiFlashStatus EF_spiFlashEraseBlock(u32 address)
{
	EF_spiFlashStatus status=SPIFLASH_STATUS_NOERR;
	while( EF_spiFlashIsBusy())	__nop();
	if (address>MX66_Flash_Size) 			
		return SPIFLASH_STATUS_WRONGADDR;
	// Enable writing
	if (!EF_spiFlashIsWriteEnabled()) 	
		EF_spiFlashWriteEnable();
	SPIFLASH_CS_L;
	//Request block erase and send the address in 24 bit
	EF_spiFlashRW(SPIFLASH_CMD_BE4B);
	EF_spiFlashRW(address >> 24);
	EF_spiFlashRW(address >> 16);
	EF_spiFlashRW(address >> 8);
	EF_spiFlashRW(address);
	SPIFLASH_CS_H;
	//wait till memory finish
	status=EF_spiFlashWaitForReady(MX66_BlockEraseCycleTime);
	EF_spiFlashWriteDisable();
	return status;
}

EF_spiFlashStatus EF_spiFlashEraseSector(u32 address)
{
	EF_spiFlashStatus status=SPIFLASH_STATUS_NOERR;
	while( EF_spiFlashIsBusy())	__nop();
	if (address>MX66_Flash_Size) 		
		return SPIFLASH_STATUS_WRONGADDR;
	// Enable writing
	if (!EF_spiFlashIsWriteEnabled()) 	EF_spiFlashWriteEnable();
	SPIFLASH_CS_L;
	//Request sector erase and send the address in 24 bit
	EF_spiFlashRW(SPIFLASH_CMD_SE4B);
	EF_spiFlashRW(address >> 24);
	EF_spiFlashRW(address >> 16);
	EF_spiFlashRW(address >> 8);
	EF_spiFlashRW(address);
	SPIFLASH_CS_H;
	//wait till memory finish
	status=EF_spiFlashWaitForReady(MX66_SectorEraseCycleTime);
	EF_spiFlashWriteDisable();
	return status;
}

#define FASTREADMODE_ON
EF_spiFlashStatus EF_spiFlashRead(u32 address, u8 *buffer, u32 byteLength)
{
	u32 i=0;
	while( DMA_Tx_Busy == 1) __nop();
	while( EF_spiFlashIsBusy())	__nop();
	if ((address+byteLength)>MX66_Flash_Size)   
		return   SPIFLASH_STATUS_WRONGADDR;
	SPIFLASH_CS_L;
	//send the Read request and the address in 24 bit + 1 dummy byte
#ifdef FASTREADMODE_ON
	EF_spiFlashRW(SPIFLASH_CMD_FASTREAD4B);
#else
	EF_spiFlashRW(SPIFLASH_CMD_READ4B);
#endif
	EF_spiFlashRW(address >> 24);
	EF_spiFlashRW(address >> 16);
	EF_spiFlashRW(address >> 8);
	EF_spiFlashRW(address);
#ifdef FASTREADMODE_ON
	EF_spiFlashRW(SPIFLASH_CMD_DUMMY);//For FASTREAD mode.
#endif
	for(i=0;i<byteLength;i++)
	{
		buffer[i]=EF_spiFlashRW(SPIFLASH_CMD_DUMMY);
	}
	SPIFLASH_CS_H;
  return SPIFLASH_STATUS_NOERR;
}

EF_spiFlashStatus EF_spiFlashPageWrite(u32 address, u8 *buffer,u32 byteLength)
{
	u16 i;
	EF_spiFlashStatus status=SPIFLASH_STATUS_NOERR;
	while( EF_spiFlashIsBusy())	__nop();
	if ((address+byteLength)>MX66_Flash_Size) 	 
		return SPIFLASH_STATUS_WRONGADDR;
	if (((address&0x000000FF)+byteLength)>MX66_Page_Offset) 
		return SPIFLASH_STATUS_PAGEEX;
	// Enable writing
	if (!EF_spiFlashIsWriteEnabled()) 	
		EF_spiFlashWriteEnable();
	__nop();
	SPIFLASH_CS_L;
	//send the Write request and the address in 24 bit
	EF_spiFlashRW(SPIFLASH_CMD_PP4B);
	EF_spiFlashRW(address >> 24);
	EF_spiFlashRW(address >> 16);
	EF_spiFlashRW(address >> 8);
	EF_spiFlashRW(address);
	for(i=0;i<byteLength;i++)
	{
		EF_spiFlashRW(buffer[i]);
	}
	SPIFLASH_CS_H;
	__nop();
	//wait till memory finish
	status=EF_spiFlashWaitForReady(MX66_PageProgramCycleTime*2);
	__nop();
	EF_spiFlashWriteDisable();
	__nop();
	return status;
}

//Write any number of data using PageWrite function
EF_spiFlashStatus EF_spiFlashWrite(u32 address, u8 *buffer,u32 byteLength)
{
	u8 NumOfPage = 0;
	u8 NumOfSingle = 0;
	u8 Addr = 0;
	u8 count = 0;
	u8 temp = 0;
	
	while( EF_spiFlashIsBusy())	__nop();
	if ((address+byteLength)>MX66_Flash_Size) 	 
		return SPIFLASH_STATUS_WRONGADDR;
	
	Addr = address % MX66_Page_Size;
	count = MX66_Page_Size - Addr;
	NumOfPage = byteLength / MX66_Page_Size;
	NumOfSingle = byteLength % MX66_Page_Size;
	__nop();
	
	if(Addr == 0)
	{
		if(NumOfPage == 0)
		{
			EF_spiFlashPageWrite(address,buffer,(u16)byteLength);
		}
		else
		{
			while(NumOfPage--)
			{
				EF_spiFlashPageWrite(address,buffer,MX66_Page_Size);
				address += MX66_Page_Size;
				buffer +=MX66_Page_Size;
			}
			EF_spiFlashPageWrite(address,buffer,NumOfSingle);
		}
	}
	else
	{
		if(NumOfPage == 0)
		{
			if(NumOfSingle > count)
			{
				temp=NumOfSingle-count;
				EF_spiFlashPageWrite(address,buffer,count);
				address += count;
				buffer +=count;
				EF_spiFlashPageWrite(address,buffer,temp);
			}
			else
			{
				EF_spiFlashPageWrite(address,buffer,byteLength);
			}
		}
		else
		{
			byteLength -= count;
			NumOfPage = byteLength / MX66_Page_Size;
			NumOfSingle = byteLength % MX66_Page_Size;
			
			EF_spiFlashPageWrite(address,buffer,count);
			address += count;
			buffer += count;
			
			while(NumOfPage--)
			{
				EF_spiFlashPageWrite(address,buffer,MX66_Page_Size);
				address += MX66_Page_Size;
				buffer += MX66_Page_Size;
			}
			if(NumOfSingle !=0)
			{
				EF_spiFlashPageWrite(address,buffer,NumOfSingle);
			}
		}
	}
	return SPIFLASH_STATUS_NOERR;
}

void EF_spiFlashSoftReset(void)
{
	while( EF_spiFlashIsBusy() ) __nop();
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_RSTEN);
	SPIFLASH_CS_H;
	DelayMs(10);
	SPIFLASH_CS_L;
	EF_spiFlashRW(SPIFLASH_CMD_RSTEN);
	SPIFLASH_CS_H;
	DelayMs(400);
}

void EF_spiFlashDmaPageWrite(u32 address, u8 *buff, u16 length)
{
	// Enable writing
	EF_spiFlashWriteEnable();
	SPIFLASH_CS_L;
	//send the Write request and the address in 24 bit
	EF_spiFlashRW(SPIFLASH_CMD_PP4B);
	EF_spiFlashRW(address >> 24);
	EF_spiFlashRW(address >> 16);
	EF_spiFlashRW(address >> 8);
	EF_spiFlashRW(address);
	EF_spiFlashDmaTx(buff,length);
}

void EF_spiFlashDmaTx(u8* buff,u16 length)
{
	while(DMA_Tx_Busy == 1)
		__nop();
	DMA_Tx_Busy = 1;
	DMA_Cmd(DMA1_Stream4,DISABLE);
	DMA1_Stream4->M0AR = (u32)buff;
	SPI_I2S_DMACmd(SPIFLASH_SPI,SPI_I2S_DMAReq_Tx,ENABLE);
	DMA_Cmd(DMA1_Stream4,ENABLE);
}

void DMA1_Stream4_IRQHandler(void)
{
	u16 i;
	u8 tmp;
	if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4) == SET)
	{
		TP3_L;
		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
		for(i=0;i<100;i++)
		{
			__nop();
		}
		DMA_Cmd(DMA1_Stream4,DISABLE);
		SPI_I2S_DMACmd(SPIFLASH_SPI,SPI_I2S_DMAReq_Tx,DISABLE);
		tmp=SPI_I2S_ReceiveData(SPIFLASH_SPI);
		SPIFLASH_CS_H;
		DMA_Tx_Busy = 0;
		TP3_H;
	}
	else if(DMA_GetITStatus(DMA1_Stream4,DMA_IT_TEIF4) == SET)
	{
		__nop();
		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TEIF4);
	}
}















