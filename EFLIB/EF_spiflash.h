#ifndef _EF_SPIFlash_H
#define _EF_SPIFlash_H

#include "stm32f4xx.h"
#include "EF_tick.h"
#include "led.h"

#define DATA_START_ADDRESS (u32)0x00

#define MX66L51235F
#ifdef  MX66L51235F

#define MX66_Baudrate 	SPI_BaudRatePrescaler_8 //84MHz/8-->10.5MHz  seems affects DMA write
#define DISK_OFFSET 		1024 //sector

/***Hardware relevent***/
/***SPI***/
#ifndef SPIFLASH_SPI
	#define SPIFLASH_USE_SPI2	//SPIFLASH_USE_SPI2 SPIFLASH_USE_SPI3
	#ifdef 	SPIFLASH_USE_SPI1
		#define SPIFLASH_SPI 								SPI1
		#define SPIFLASH_SPI_RCC						RCC_APB2Periph_SPI1
		#define SPIFLASH_SPI_AF							GPIO_AF_SPI1
		#define SPIFLASH_SPI_PORT						GPIOA
		#define SPIFLASH_SPI_GPIO_RCC				RCC_AHB1Periph_GPIOA
		#define SPIFLASH_SPI_PIN						GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7 //(SCK|MISO|MOSI)
		#define SPIFLASH_SPI_SCK_PINSOURCE 	GPIO_PinSource5
		#define SPIFLASH_SPI_MISO_PINSOURCE GPIO_PinSource6		
		#define SPIFLASH_SPI_MOSI_PINSOURCE GPIO_PinSource7
	#endif
	#ifdef  SPIFLASH_USE_SPI2
		#define SPIFLASH_SPI 								SPI2
		#define SPIFLASH_SPI_RCC						RCC_APB1Periph_SPI2
		#define SPIFLASH_SPI_AF							GPIO_AF_SPI2
		#define SPIFLASH_SPI_PORT						GPIOB
		#define SPIFLASH_SPI_GPIO_RCC				RCC_AHB1Periph_GPIOB		
		#define SPIFLASH_SPI_PIN						GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15 //(SCK|MISO|MOSI)
		#define SPIFLASH_SPI_SCK_PINSOURCE 	GPIO_PinSource13
		#define SPIFLASH_SPI_MISO_PINSOURCE GPIO_PinSource14	
		#define SPIFLASH_SPI_MOSI_PINSOURCE GPIO_PinSource15
	#endif
	#ifdef 	SPIFLASH_USE_SPI3
		#define SPIFLASH_SPI 								SPI3
		#define SPIFLASH_SPI_RCC						RCC_APB1Periph_SPI3
		#define SPIFLASH_SPI_AF							GPIO_AF_SPI3		
		#define SPIFLASH_SPI_PORT						GPIOC
		#define SPIFLASH_SPI_GPIO_RCC				RCC_AHB1Periph_GPIOC		
		#define SPIFLASH_SPI_PIN						GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12 //(SCK|MISO|MOSI)
		#define SPIFLASH_SPI_SCK_PINSOURCE 	GPIO_PinSource10
		#define SPIFLASH_SPI_MISO_PINSOURCE GPIO_PinSource11	
		#define SPIFLASH_SPI_MOSI_PINSOURCE GPIO_PinSource12
	#endif
#endif

/***CS***/
#ifndef SPIFLASH_CS_PIN
	#define SPIFLASH_CS_PORT				GPIOB
	#define SPIFLASH_CS_PIN 				GPIO_Pin_12
	#define SPIFLASH_CS_GPIO_RCC		RCC_AHB1Periph_GPIOB
	#define SPIFLASH_CS_H						__nop();GPIO_SetBits(SPIFLASH_CS_PORT,SPIFLASH_CS_PIN);__nop()
	#define SPIFLASH_CS_L						__nop();GPIO_ResetBits(SPIFLASH_CS_PORT,SPIFLASH_CS_PIN);__nop()
#endif

/***Memory ID***/
#define 	MX66_DeviceID			0xC2201A
#define 	MX66_ElectronicID	0x19
/***Flash Related Parameters***/
#define 	MX66_Flash_Size						0x4000000  		  // 512Mbits (64MB)
#define   MX66_Block_64K_Offset    	0x10000    		// 64K Block size
#define		MX66_Block_32K_Offset			0x08000				// 32K Block size
#define   MX66_Sector_Offset    		0x01000      	//  4K Sector size
#define   MX66_Page_Offset      		0x00100      	// 256 Byte Page size
#define   MX66_Block_64K_Num				(MX66_Flash_Size / MX66_Block_64K_Offset)
#define		MX66_Block_32K_Num				(MX66_Flash_Size / MX66_Block_32K_Offset)
#define		MX66_Sector_Num						(MX66_Flash_Size / MX66_Sector_Size)
#define   MX66_Pages_In_Block_64K		(MX66_Block_64K_Offset / MX66_Page_Offset)
#define		MX66_Pages_In_Block_32K		(MX66_Block_32K_Offset / MX66_Page_Offset)
#define		MX66_Page_Size						MX66_Page_Offset
#define		MX66_Sector_Size					MX66_Sector_Offset
#define		MX66_Block_Size						MX66_Block_64K_Offset
/***Flash information define***/
#define   MX66_WriteStatusRegCycleTime      40	//40ms
#define   MX66_ByteProgramCycleTime					 1		//1ms (300us)
#define   MX66_PageProgramCycleTime        	 3		//3ms
#define   MX66_SectorEraseCycleTime        200	//200ms
#define   MX66_BlockEraseCycleTime        2000	//2s
#define   MX66_ChipEraseCycleTime       300000	//300s
/***Operating command***/
#define		SPIFLASH_CMD_RDID				  0x9F    //RDID (Read Device Identification)
#define 	SPIFLASH_CMD_DUMMY				0xFF		//DUMMY COMMAND (used to generate clock and receive data)
/***Register commands***/
#define   SPIFLASH_CMD_RDSR      	  0x05    //RDSR (Read Status Register)
#define 	MEM_WIP_MASK				      0x01	//Mask to get the (WIP) Work In Progress bit
#define 	MEM_WEL_MASK				      0x02	//Mask to get the (WEL) Write Enable Latch bit
/***W/R commands***/
#define   SPIFLASH_CMD_WREN     		0x06    //WREN (Write Enable)
#define   SPIFLASH_CMD_WRDI     		0x04    //WRDI (Write Disable)
/***3Byte Address Command***/
#define		SPIFLASH_CMD_READ				  0x03		//Read
#define   SPIFLASH_CMD_FASTREAD		  0x0B    //Fast read
#define   SPIFLASH_CMD_CE       		0x60    //CE (Chip Erase)
#define   SPIFLASH_CMD_BE       		0x52    //BE (Block Erase 64KB)
#define		SPIFLASH_CMD_BE32K				0xD8		//BE32K (Block Erase 32KB)
#define   SPIFLASH_CMD_SE       		0x20    //SE (Sector Erase 4KB)
#define   SPIFLASH_CMD_PP       		0x02    //PP (page program)
/***4Byte Address Command***/
#define		SPIFLASH_CMD_READ4B				0x13	//Read data byte by 4 byte address
#define		SPIFLASH_CMD_FASTREAD4B		0x0C	//Fast read data byte by 4 byte address
#define		SPIFLASH_CMD_PP4B					0x12	//Page Program
#define		SPIFLASH_CMD_BE4B					0xDC	//Block Erase 64KB
#define		SPIFLASH_CMD_BE32K4B			0x5C	//Block Erase 32KB
#define		SPIFLASH_CMD_SE4B					0x21	//Sector Erase 4KB
#define		SPIFLASH_CMD_EN4B					0xB7	//EN4B (enter 4-byte mode and set 4BYTE bit as "1")
#define		SPIFLASH_CMD_EX4B					0xE9	//EX4B (exit 4-byte mode and clear 4BYTE bit as "0")
#define		SPIFLASH_CMD_RSTEN				0x66	//RESETENABLE
#define		SPIFLASH_CMD_RST					0x99	//RESET
/***memory status***/
typedef enum{
	SPIFLASH_STATUS_NOERR=0,
	SPIFLASH_STATUS_BUSY=1,
	SPIFLASH_STATUS_WRONGADDR=2,
	SPIFLASH_STATUS_PAGEEX=3,
	SPIFLASH_STATUS_TIMEOUT=4
} EF_spiFlashStatus;

/***Function***/
void EF_spiFlashInit(void);
u32 EF_spiFlashGetID(void);
EF_spiFlashStatus EF_spiFlashEraseAll(void);
EF_spiFlashStatus EF_spiFlashEraseBlock(u32 address);
EF_spiFlashStatus EF_spiFlashEraseSector(u32 address);
EF_spiFlashStatus EF_spiFlashRead(u32 address, u8 *buffer, u32 byteLength);
EF_spiFlashStatus EF_spiFlashPageWrite(u32 address, u8 *buffer,u32 byteLength);
EF_spiFlashStatus EF_spiFlashWrite(u32 address, u8 *buffer,u32 byteLength);
void EF_spiFlashDmaPageWrite(u32 address, u8* buff,u16 length);
void EF_spiFlashSoftReset(void);
u8 EF_spiFlashIsBusy(void);
void EF_spiFlashWriteEnable(void);
void EF_spiFlashWriteDisable(void);
u8 EF_spiFlashIsWriteEnabled(void);
EF_spiFlashStatus EF_spiFlashWaitForReady(u32 waitTime);
#endif
#endif


