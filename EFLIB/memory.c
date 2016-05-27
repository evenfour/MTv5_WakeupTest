#include "memory.h"

#define tick_GetTimeMs() EF_TickGetTimeMs()
//Memory Initialization
//GPIO, SPI1, and DMA1 clock configuration
void memory_ClockInit(void)
{
	RCC_AHB1PeriphClockCmd(MEM_SPI_PORT_RCC | MEM_CS_RCC,ENABLE);
	RCC_APB2PeriphClockCmd(MEM_SPI_RCC, ENABLE);
}

//SPI1 GPIO configuration
void memory_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	
	// Memory CS pin
	GPIO_InitStructure.GPIO_Pin = MEM_SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_Init(MEM_CS_PORT, &GPIO_InitStructure);
	MEM_CS_H;
	
	// Memory SPI pins
	GPIO_InitStructure.GPIO_Pin = MEM_SPI_SCK_PIN | MEM_SPI_MISO_PIN | MEM_SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_PinAFConfig(MEM_SPI_PORT,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(MEM_SPI_PORT,GPIO_PinSource6,GPIO_AF_SPI1);
	GPIO_PinAFConfig(MEM_SPI_PORT,GPIO_PinSource7,GPIO_AF_SPI1);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(MEM_SPI_PORT, &GPIO_InitStructure);
}

//SPI1 configuration
void memory_SPIInit(void)
{
	//configure SPI1 in Mode 0
	SPI_InitTypeDef SPI_InitStruct;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;		// CPOL = 0 --> clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;	//CPHA = 0 --> data is sampled at the first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // ABP2 is running at 42MHz
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	//SPI_InitStruct.
	SPI_Init(SPI1,&SPI_InitStruct);
	SPI_Cmd(SPI1,ENABLE);
}


void memory_Init(void)
{
	memory_ClockInit(); //Clock
	memory_GPIOInit();	//GPIO
	memory_SPIInit();		//SPI
	DelayMs(5);
	memory_4BModeOn(); 	//4Byte Mode
}

u8 dummyMEMPage[256]= {
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		// 32
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		// 64
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		// 96
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		//128
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		//160
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		//192
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		//224
					    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0			//256
					};

//SPI basic W/R function
u8 memory_sendByte(u8 data)
{
	u32 timeout;
	
	timeout=10000;
	while(!(SPI1->SR & SPI_I2S_FLAG_TXE) && timeout>0)
	{
		timeout--; 	// wait until transmit complete
	}
	SPI1->DR = data; 															// write data to be transmitted to the SPI data register
	
	// wait until receive complete
	timeout=10000;
	while(!(SPI1->SR & SPI_I2S_FLAG_RXNE)&& timeout>0)
	{
		timeout--; 	// wait until transmit complete		
	}
	data = SPI1->DR;															// get data received
	return data;
}

//Verify Device ID
void memory_VerifyID(u8* memID)
{
	MEM_CS_L;
	//send the DeviceID request
	memory_sendByte(MEM_CMD_RDID);
	//get the 24 bit value from 3 bytes
	* memID 		= memory_sendByte(MEM_CMD_DUMMY);
	*(memID+1) 	= memory_sendByte(MEM_CMD_DUMMY);
	*(memID+2) 	= memory_sendByte(MEM_CMD_DUMMY);

	MEM_CS_H;
}

//WEL Set
void memory_WriteEnable()
{
	MEM_CS_L;
	memory_sendByte(MEM_CMD_WREN);
	MEM_CS_H;
}
//WEL Reset
void memory_WriteDisable()
{
	MEM_CS_L;
	memory_sendByte(MEM_CMD_WRDI);
	MEM_CS_H;
}
//Get Status Register
u8 memory_GetStatusReg()
{
	u8 statusReg=0;
	MEM_CS_L;
	memory_sendByte(MEM_CMD_RDSR);
	statusReg = memory_sendByte(MEM_CMD_DUMMY);
	MEM_CS_H;
	return statusReg;
}

void memory_4BModeOn()
{
	MEM_CS_L;
	memory_sendByte(MEM_CMD_EN4B);
	MEM_CS_H;
}

void memory_4BModeOff()
{
	MEM_CS_L;
	memory_sendByte(MEM_CMD_EX4B);
	MEM_CS_H;
}

u8 memory_IsBusy()
{
	return ((memory_GetStatusReg() & MEM_WIP_MASK) == MEM_WIP_MASK);
}

u8 memory_IsWriteEnabled()
{
	return ((memory_GetStatusReg() & MEM_WEL_MASK) == MEM_WEL_MASK);
}

u32 memory_WaitForReady(u32 waitTime)
{
	u32 startTime = tick_GetTimeMs();
	while (memory_IsBusy())
	{
		if ( (tick_GetTimeMs()-startTime) > waitTime )
			return MEMERR_TIMEEXCEEDED;
	}
	return tick_GetTimeMs()-startTime;
}

u16 memory_EraseAll(u8 waitToFinish)
{
	while(memory_IsBusy())	;
	//if (memory_IsBusy())			return MEMERR_BUSY;
	if (!memory_IsWriteEnabled())	memory_WriteEnable();
	MEM_CS_L;
	// Request chip erase
	memory_sendByte(MEM_CMD_CE);
	MEM_CS_H;
	//wait till memory finish
	if (waitToFinish)			 memory_WaitForReady(ChipEraseCycleTime);
	return MEMERR_NOERR;
}

u16 memory_EraseBlock(u32 memAddress, u8 waitToFinish)
{
	while(memory_IsBusy()) ;
	//if (memory_IsBusy()) 			return MEMERR_BUSY;
	if (memAddress>FlashSize) 	return MEMERR_WRONGADDRESS;
	// Enable writing
	if (!memory_IsWriteEnabled()) 	memory_WriteEnable();
	//delayMs(1);
	MEM_CS_L;
	//Request block erase and send the address in 24 bit
	memory_sendByte(MEM_CMD_BE4B);
	memory_sendByte(memAddress >> 24);
	memory_sendByte(memAddress >> 16);
	memory_sendByte(memAddress >> 8);
	memory_sendByte(memAddress);
	MEM_CS_H;
	//wait till memory finish
	if (waitToFinish) memory_WaitForReady(BlockEraseCycleTime);
	memory_WriteDisable();
	return MEMERR_NOERR;
}

u16 memory_EraseSector(u32 memAddress, u8 waitToFinish)
{
	while (memory_IsBusy()) ;
	//return MEMERR_BUSY;
	if (memAddress>FlashSize) 	return MEMERR_WRONGADDRESS;
	// Enable writing
	if (!memory_IsWriteEnabled()) 	memory_WriteEnable();
	MEM_CS_L;
	//Request sector erase and send the address in 24 bit
	memory_sendByte(MEM_CMD_SE4B);
	memory_sendByte(memAddress >> 24);
	memory_sendByte(memAddress >> 16);
	memory_sendByte(memAddress >> 8);
	memory_sendByte(memAddress);
	MEM_CS_H;
	//wait till memory finish
	if (waitToFinish) memory_WaitForReady(SectorEraseCycleTime);
	memory_WriteDisable();
	return MEMERR_NOERR;
}

u16 memory_Write(u32 memAddress, u8 *bytesToSend, u16 byteLength, u8 waitToFinish)
{
	u8 i;
	//if (memory_IsBusy()) return MEMERR_BUSY;
	while (memory_IsBusy())	;
	//if (memAddress>FlashSize) 	return MEMERR_WRONGADDRESS;
	//if (((memAddress&0x000000FF)+byteLength)>Page_Offset) return MEMERR_PAGEEXCEEDED;
	// Enable writing
	if (!memory_IsWriteEnabled()) 	memory_WriteEnable();
	MEM_CS_L;
	//send the Write request and the address in 24 bit
	memory_sendByte(MEM_CMD_PP4B);
	memory_sendByte(memAddress >> 24);
	memory_sendByte(memAddress >> 16);
	memory_sendByte(memAddress >> 8);
	memory_sendByte(memAddress);
	for(i=0;i<byteLength;i++)
	{
		memory_sendByte(bytesToSend[i]);
	}
	MEM_CS_H;
	//wait till memory finish
	if (waitToFinish) memory_WaitForReady(PageProgramCycleTime);
	memory_WriteDisable();
	return MEMERR_NOERR;
}

//#define FASTREADMODE_ON
u16 memory_Read(u32 memAddress, u8 *buffer, u32 byteLength)
{
	int i;
	while(memory_IsBusy())	;
	//if (memory_IsBusy()) return MEMERR_BUSY;
	//if (memAddress>FlashSize) return MEMERR_WRONGADDRESS;
	MEM_CS_L;
	//send the Read request and the address in 24 bit + 1 dummy byte
#ifdef FASTREADMODE_ON
	memory_sendByte(MEM_CMD_FASTREAD4B);
#else
	memory_sendByte(MEM_CMD_READ);
#endif
	memory_sendByte(memAddress >> 24);
	memory_sendByte(memAddress >> 16);
	memory_sendByte(memAddress >> 8);
	memory_sendByte(memAddress);
#ifdef FASTREADMODE_ON
	memory_sendByte(MEM_CMD_DUMMY);//For FASTREAD mode.
#endif
	for(i=0;i<byteLength;i++)
	{
		buffer[i]=memory_sendByte(MEM_CMD_DUMMY);
	}
	MEM_CS_H;
    return MEMERR_NOERR;
}

/**                                                                                    **
 ***    Predefined Functions - END                                                    ***
 ****************************************************************************************/



