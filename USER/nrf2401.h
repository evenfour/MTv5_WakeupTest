#ifndef NRF2401_H_
#define NRF2401_H_

#include "stm32f4xx.h"
#include "EF_tick.h"

#define NRF_MAX_TX  	0x10  //INT Max
#define NRF_TX_OK   	0x20  //INT TX
#define NRF_RX_OK   	0x40  //INT RX

#define NRF_READ_REG        0x00  //Read configuration register
#define NRF_WRITE_REG       0x20  //Write configuration register
#define NRF_RD_RX_PLOAD     0x61  //Read RX data
#define NRF_WR_TX_PLOAD     0xA0  //Write TX data
#define NRF_FLUSH_TX        0xE1  //Flush TX FIFO in T mode
#define NRF_FLUSH_RX        0xE2  //Flush RX FIFO in R mode
#define NRF_REUSE_TX_PL     0xE3  //Reuse last data package
#define NRF_NOP             0xFF  //NOP operation

//SPI(REG)
#define NRF_CONFIG          0x00  //
#define NRF_EN_AA           0x01  //
#define NRF_EN_RXADDR       0x02  //
#define NRF_SETUP_AW        0x03  //
#define NRF_SETUP_RETR      0x04  //
#define NRF_RF_CH           0x05  //
#define NRF_RF_SETUP        0x06  //
#define NRF_STATUS          0x07  //
#define NRF_OBSERVE_TX      0x08  //
#define NRF_CD              0x09  //
#define NRF_RX_ADDR_P0      0x0A  //
#define NRF_RX_ADDR_P1      0x0B  //
#define NRF_RX_ADDR_P2      0x0C  //
#define NRF_RX_ADDR_P3      0x0D  //
#define NRF_RX_ADDR_P4      0x0E  //
#define NRF_RX_ADDR_P5      0x0F  //
#define NRF_TX_ADDR         0x10  //
#define NRF_RX_PW_P0        0x11  //
#define NRF_RX_PW_P1        0x12  //
#define NRF_RX_PW_P2        0x13  //
#define NRF_RX_PW_P3        0x14  //
#define NRF_RX_PW_P4        0x15  //
#define NRF_RX_PW_P5        0x16  //
#define NRF_FIFO_STATUS     0x17  //
                              //
#define NRF_ADR_WIDTH    3   //Length of address
#define NRF_PLOAD_WIDTH  4  //Length of user data

#define NRF24L01_CE_Pin   GPIO_Pin_7  	//CE PC7
#define NRF24L01_CSN_Pin  GPIO_Pin_6  	//SPI CSN PC6
#define NRF24L01_IRQ_Pin  GPIO_Pin_8  	//INT PC8
#define NRF24L01_CLK		GPIO_Pin_13 		// PB13
#define NRF24L01_MISO 	GPIO_Pin_14 		// PB14
#define NRF24L01_MOSI 	GPIO_Pin_15 		// PB15


//24L01 CE Pin Operation
#define NRF24L01_CE_H 	GPIO_SetBits(GPIOC, NRF24L01_CE_Pin)
#define NRF24L01_CE_L 	GPIO_ResetBits(GPIOC, NRF24L01_CE_Pin)

//24L01 SPI CSN Pin Operation
#define NRF24L01_CSN_H 	GPIO_SetBits(GPIOC, NRF24L01_CSN_Pin)
#define NRF24L01_CSN_L	GPIO_ResetBits(GPIOC, NRF24L01_CSN_Pin)

//24L01 IRQ Pin Operation
#define NRF24L01_IRQ  	GPIO_ReadInputDataBit(GPIOC,NRF24L01_IRQ_Pin)

void NRF24L01_Init(void);

void RX_Mode(void);
u8 TX_Mode(u8 *txbuf);

u8 NRF24L01_Write_Buf(u8 *pBuf, u8 u8s);
u8 NRF24L01_Read_Buf(u8 *pBuf, u8 u8s);

u8 NRF24L01_Write_Reg(u8 reg, u8 value);
u8 NRF24L01_Read_Reg(u8 reg);

u8 NRF24L01_Write_Addr(u8 reg,u8 *pBuf,u8 len);
u8 NRF24L01_Read_Addr(u8 reg,u8 *pBuf,u8 len);

u8 NRF24L01_TxPacket(u8 *txbuf);
u8 NRF24L01_RxPacket(u8 *rxbuf);

u8 SPI_RW_Byte(SPI_TypeDef* SPIx,unsigned char Byte);

u8 NRF24L01_Check(void);

void SPI2_Init(void);

void NRF24L01_Flush_TX(void);
void NRF24L01_Flush_RX(void);

#endif











