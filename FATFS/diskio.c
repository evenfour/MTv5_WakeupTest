/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "EF_spiflash.h"

/* Definitions of physical drive number for each drive */
#define ATA									0	/* Example: Map ATA harddisk to physical drive 0 */
#define FATFS_SPI_FLASH			1	/* Example: Map FATFS_SPI_FLASH/SD card to physical drive 1 */
#define USB									2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case ATA :
		return STA_OK;
	
	case FATFS_SPI_FLASH :
		return STA_OK;
	
	case USB :
		return STA_OK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case ATA :
		return STA_OK;
	case FATFS_SPI_FLASH:
		if(EF_spiFlashGetID()==(u32)MX66_DeviceID)
			return STA_OK;
		break;
	case USB :
		return STA_OK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
	case ATA :
		return RES_OK;
	
	case FATFS_SPI_FLASH :
		if( EF_spiFlashRead((sector+DISK_OFFSET)<<12,buff,count<<12) == SPIFLASH_STATUS_NOERR)
			return RES_OK;
	case USB :
		return RES_OK;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	u32 write_addr;
	if(!count) return RES_PARERR;
	
	switch (pdrv) {
	case ATA :
		return RES_OK;

	case FATFS_SPI_FLASH :
		
		write_addr=(sector+DISK_OFFSET)<<12;
		if(EF_spiFlashEraseSector(write_addr) == SPIFLASH_STATUS_NOERR )
		{
			if( EF_spiFlashWrite(write_addr,(u8 *)buff,count<<12) == SPIFLASH_STATUS_NOERR )
			{
				return RES_OK;
			}
			else
			{
				return RES_ERROR;
			}
		}
		else
		{
			return RES_PARERR;
		}
		
	case USB :
		return RES_OK;
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {
	case ATA :
				return RES_OK;

	case FATFS_SPI_FLASH :

		// Process of the command for the FATFS_SPI_FLASH/SD card
		switch (cmd)
		{
			case CTRL_SYNC:
				return RES_OK;
			
			case GET_SECTOR_COUNT:
				*(DWORD *)buff = 	(u32)MX66_Sector_Num-DISK_OFFSET;
				return RES_OK;
			
			case GET_SECTOR_SIZE:
				*(WORD *)buff = 	(u32)MX66_Sector_Size;
				return RES_OK;
			
			case GET_BLOCK_SIZE:
				*(DWORD *)buff = 1;
				return RES_OK;
		}
		return RES_PARERR;

	case USB :
		return RES_OK;
	}

	return RES_PARERR;
}

DWORD get_fattimes(void) 
{
	return 0;
}

#endif
