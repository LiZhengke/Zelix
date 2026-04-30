/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */
/* Example: Declarations of the platform and disk functions in the project */
#include "ata.h"
int RAM_disk_status(void);
static int MMC_disk_read(void *buf, uint32_t lba,  uint32_t count);

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map FTL to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */
#define DEV_RAM	    3	/* Map RAM disk to physical drive 3 */

#define SECTOR_SIZE 512
#define TOTAL_SECTORS (16 * 1024 * 1024 / SECTOR_SIZE)  /* 假设 16MB disk */
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = 0;
	// int result;

	switch (pdrv) {
	case DEV_RAM :
		// result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		// result = MMC_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		// result = USB_disk_initialize();

		// translate the reslut code here

		return stat;
	}
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	int result = MMC_disk_read(buff, sector, count);

	return result == 0 ? RES_OK : RES_ERROR;


}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_PARERR;
	// int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		// result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		// result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_USB :
		// translate the arguments here

		// result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;
	// int result;

	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card
		switch (cmd) {
			case CTRL_SYNC:
				// 确保写入完成（PIO 模式一般不需要额外处理）
				return RES_OK;

			case GET_SECTOR_COUNT:
				*(LBA_t*)buff = TOTAL_SECTORS;
				return RES_OK;

			case GET_SECTOR_SIZE:
				*(WORD*)buff = SECTOR_SIZE;
				return RES_OK;

			case GET_BLOCK_SIZE:
				// 擦除块大小（单位：sector）
				*(DWORD*)buff = 1;
				return RES_OK;
			}

		return res;

	case DEV_USB :

		// Process of the command the USB drive

		return res;
	}

	return RES_PARERR;
}

static int MMC_disk_read(void *buf, uint32_t lba,  uint32_t count) {
    uint8_t *p = buf;

    for (uint32_t i = 0; i < count; i++) {
        if (ata_read_sector(lba + i, p + i * 512) < 0)
            return -1;
    }

    return 0;
}
