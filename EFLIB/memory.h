#ifndef MEMORY_H_
#define MEMORY_H_

#include "stm32f4xx.h"
#include "EF_tick.h"

#define MX66L51235F
#define FASTREADMODE_ON


#define MEM_CS_H	GPIO_SetBits(MEM_CS_PORT,MEM_SPI_CS_PIN)
#define MEM_CS_L	GPIO_ResetBits(MEM_CS_PORT,MEM_SPI_CS_PIN)

//clock
#define MEM_SPI_RCC				RCC_APB2Periph_SPI1		//Memory SPI clock (SPI1)
#define MEM_SPI_PORT_RCC	RCC_AHB1Periph_GPIOA	//Memory SPI GPIO clock
#define MEM_CS_RCC				RCC_AHB1Periph_GPIOB	//Memory CS	 GPIO clock

//port
#define MEM_SPI_PORT			GPIOA									//Memory SPI port (SPI1)
#define MEM_CS_PORT				GPIOB									//Memory SPI chip select (CS)

//pin
#define MEM_SPI_SCK_PIN		GPIO_Pin_5 						//Memory SPI clock pin
#define MEM_SPI_MISO_PIN 	GPIO_Pin_6						//Memory SPI MISO pin
#define MEM_SPI_MOSI_PIN 	GPIO_Pin_7						//Memory SPI MOSI pin
#define MEM_SPI_CS_PIN 		GPIO_Pin_6						//Memory SPI CS pin


#ifdef 		MX66L51235F
	// Memory ID
	#define 	DeviceID		0xC2201A
	#define 	ElectronicID	0x19
	// Flash Related Parameter Define
	#define 	FlashSize		0x4000000  		// 64MBytes
	#define   Block_64K_Offset    0x10000    	// 64K Block size
	#define		Block_32K_Offset	0x8000		// 32K Block size
	#define   Sector_Offset    	0x1000      	// 4K Sector size
	#define   Page_Offset      	0x0100      	// 256 Byte Page size
	#define   Block_64K_Num		(FlashSize / Block_64K_Offset)
	#define		Block_32K_Num		(FlahsSize / Block_32K_Offset)
	#define   Pages_In_Block_64K	(Block_64K_Offset / Page_Offset)
	#define		Pages_In_Block_32K	(Block_32K_Offset / Page_Offset)
	// Flash information define
	#define   FlashFullAccessTime         300	//100ms
	#define   WriteStatusRegCycleTime     300	//100ms
	#define   ByteProgramCycleTime		1	//300us
	#define   PageProgramCycleTime        5	//5ms
	#define   SectorEraseCycleTime        300	//300ms
	#define   BlockEraseCycleTime        3000	//2s
	#define   ChipEraseCycleTime        60000	//60s (63 Blocks)
#endif

//MX66L51232F Command
#define		MEM_CMD_RDID		0x9F    //RDID (Read Device Identification)
#define 	MEM_CMD_DUMMY		0xFF	//DUMMY COMMAND (used to generate clock and receive data)
//Register commands
#define   MEM_CMD_RDSR      	0x05    //RDSR (Read Status Register)
#define 	MEM_WIP_MASK		0x01	//Mask to get the (WIP) Work In Progress bit
#define 	MEM_WEL_MASK		0x02	//Mask to get the (WEL) Write Enable Latch bit
//WRITE/READ commands
#define   MEM_CMD_WREN     	0x06    //WREN (Write Enable)
#define   MEM_CMD_WRDI     	0x04    //WRDI (Write Disable)
//3 Byte Address Command Set
#define		MEM_CMD_READ		0x03	//Read
#define   MEM_CMD_FASTREAD	0x0B    //Fast read (we are running at 36MHz, so Normal READ is not considered)
#define   MEM_CMD_CE       	0x60    //CE (Chip Erase)
#define   MEM_CMD_BE       	0x52    //BE (Block Erase 64KB)
#define		MEM_CMD_BE32K		0xD8	//BE32K (Block Erase 32KB)
#define   MEM_CMD_SE       	0x20    //SE (Sector Erase 4KB)
#define   MEM_CMD_PP       	0x02    //PP (page program)

#ifdef 		MX66L51235F
	//4 Byte Address Command Set
	#define		MEM_CMD_READ4B		0x13	//Read data byte by 4 byte address
	#define		MEM_CMD_FASTREAD4B	0x0C	//Fast read data byte by 4 byte address
	#define		MEM_CMD_PP4B		0x12	//Page Program
	#define		MEM_CMD_BE4B		0xDC	//Block Erase 64KB
	#define		MEM_CMD_BE32K4B		0x5C	//Block Erase 32KB
	#define		MEM_CMD_SE4B		0x21	//Sector Erase 4KB
	#define		MEM_CMD_EN4B		0xB7	//EN4B (enter 4-byte mode and set 4BYTE bit as "1")
	#define		MEM_CMD_EX4B		0xE9	//EX4B (exit 4-byte mode and clear 4BYTE bit as "0")
#endif

// MEMERR_LIST
#define 	MEMERR_NOERR			0
#define 	MEMERR_BUSY				1
#define 	MEMERR_WRONGADDRESS		2
#define 	MEMERR_PAGEEXCEEDED		3
#define 	MEMERR_TIMEEXCEEDED		4

void memory_Init(void);
void memory_VerifyID(u8* memID);
u8 memory_IsBusy(void);
u16 memory_EraseAll(u8);
u16 memory_EraseBlock(u32,u8);
u16 memory_EraseSector(u32,u8);
u16 memory_Write(u32,u8*,u16,u8);
u16 memory_Read(u32,u8*,u32);
void memory_4BModeOn(void);
void memory_4BModeOff(void);

#endif


