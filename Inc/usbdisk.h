#ifndef __USBDISK_H
#define __USBDISK_H

#include "integer.h"

int USB_disk_initialize();
int USB_disk_status();  //  get status
int USB_disk_read(void* buff, DWORD sector, BYTE blockCount);
int USB_disk_write(void* buff, DWORD sector, BYTE blockCount);
int USB_disk_ioctl(BYTE cmd, void* buff);

#endif
