#ifndef MAGNET_H_
#define MAGNET_H_

#include "stm32f4xx.h"
#include "EF_Tick.h"
#include "ads8330.h"

#define MAGNET_FREQ			100

#define MAGNET_CHIPID		0x40
#define MAGNET_DATAX_LSB	0x42
#define MAGNET_DATAX_MSB	0x43
#define MAGNET_DATAY_LSB	0x44
#define MAGNET_DATAY_MSB	0x45
#define MAGNET_DATAZ_LSB	0x46
#define MAGNET_DATAZ_MSB	0x47
#define MAGNET_POWERMODE	0x4B
#define MAGNET_OPMODE		0x4C
#define MAGNET_INTSET		0x4E
#define MAGNET_REPXY		0x51
#define MAGNET_REPZ			0x52

#define MAGNET_PORT		GPIOA
#define MAGNET_CS_Pin GPIO_Pin_8  	// CS B11
#define MAGNET_DRDY		GPIO_Pin_2		// MAG DRDY B2
#define MAGNET_INT		GPIO_Pin_10		// INT ACC
#define MAGNET_MISO 	GPIO_Pin_14 	// B14
#define MAGNET_MOSI 	GPIO_Pin_15 	// B15
#define MAGNET_CLK		GPIO_Pin_13 	// B13

//BMC150 CS
#define MAGNET_CS_H 	GPIO_SetBits(MAGNET_PORT, MAGNET_CS_Pin);
#define MAGNET_CS_L 	GPIO_ResetBits(MAGNET_PORT, MAGNET_CS_Pin);

//#define MAGNET_CS_IRQ  	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2)

void MAGNET_Init(void);
u8 MAGNET_SPI_RW_Byte(SPI_TypeDef* SPIx,u8 Byte);
u8 MAGNET_Write_Reg(u8 reg, u8 value);
u8 MAGNET_Read_Reg(u8 reg);

u8 Magnet_VerifyID(void);
void Magnet_Config(void);
u16 Magnet_ReadDataX(void);
u16 Magnet_ReadDataY(void);
u16 Magnet_ReadDataZ(void);
void Magnet_Start(void);


#endif











