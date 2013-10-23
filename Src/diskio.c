/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "usbdisk.h"	/* Example: USB drive control */
//#include "atadrive.h"	/* Example: ATA drive control */
#include "sdcard.h"		/* Example: MMC/SDC control */

/* Definitions of physical drive number for each media */
#define MMC		0
#define USB		1
#define ATA		2

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result = 0;

	switch (pdrv) {
	case MMC :
		result = MMC_disk_initialize();

		// translate the result code here
		stat = (result & 0xFF);
		return RES_OK;

	case USB :
		result = USB_disk_initialize();

		// translate the result code here
		stat = (result & 0xFF);
		return stat;

	case ATA :
		//result = ATA_disk_initialize();

		// translate the result code here
		stat = (result & 0xFF);
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result = 0;

	switch (pdrv) {
	case ATA :
		//result = ATA_disk_status();

		// translate the result code here
		stat = (result & 0xFF);
		return stat;

	case MMC :
		result = MMC_disk_status();

		// translate the result code here
		stat = (result & 0xFF);
		return RES_OK;

	case USB :
		result = USB_disk_status();

		// translate the result code here
		stat = (result & 0xFF);
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;
	int result = 0;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		//result = ATA_disk_read(buff, sector, count);

		// translate the result code here
		res = (result & 0xFF);
		return res;

	case MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the result code here
		res = (result & 0xFF);
		return RES_OK;

	case USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the result code here
		res = (result & 0xFF);
		return res;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if (_USE_WRITE == 1)
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive number (0..) */
	const BYTE *buff,	/* Data to be written */    //  const keyword removed
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res;
	int result = 0;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		//result = ATA_disk_write(buff, sector, count);

		// translate the result code here
		res = (result & 0xFF);
		return res;

	case MMC :
		// translate the arguments here

		result = MMC_disk_write((void*)buff, sector, count);

		// translate the result code here
		res = (result & 0xFF);
		return res;

	case USB :
		// translate the arguments here

		result = USB_disk_write((void*)buff, sector, count);

		// translate the result code here
		res = (result & 0xFF);
		return res;
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if (_USE_IOCTL == 1)
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result = 0;

	switch (pdrv) {
	case ATA :
		// pre-process here

		//result = ATA_disk_ioctl(cmd, buff);

		// post-process here
		res = (result & 0xFF);
		return res;

	case MMC :
		// pre-process here

		result = MMC_disk_ioctl(cmd, buff);

		// post-process here
		res = (result & 0xFF);
		return res;

	case USB :
		// pre-process here

		result = USB_disk_ioctl(cmd, buff);

		// post-process here
		res = (result & 0xFF);
		return res;
	}
	return RES_PARERR;
}
#endif
