




#ifndef __SDCARD_H
#define __SDCARD_H

#include "diskio.h"

//	prototypes
DRESULT MMC_disk_initialize(void);
DRESULT MMC_disk_status(void);  //  get status
DRESULT MMC_disk_read(void* buff, DWORD sector, BYTE blockCount);
DRESULT MMC_disk_write(void* buff, DWORD sector, BYTE blockCount);
DRESULT MMC_disk_ioctl(BYTE cmd, void* buff);

#endif
