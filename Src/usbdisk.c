#include "usbdisk.h"
#include "integer.h"
// prototypes
int USB_disk_initialize();
int USB_disk_status();  //  get status
int USB_disk_read(void *buff, DWORD sector, BYTE count);
int USB_disk_write(void *buff, DWORD sector, BYTE count);
int USB_disk_ioctl(BYTE cmd, void *buff);

// functions
int USB_disk_initialize(){
    int result = 0;
    return result;
}

int USB_disk_status(){
    int result = 0;
    return result;
}

int USB_disk_read(void *buff, DWORD sector, BYTE count){
    int result = 0;
    return result;
}

int USB_disk_write(void *buff, DWORD sector, BYTE count){
    int result = 0;
    return result;
}

int USB_disk_ioctl(BYTE cmd, void *buff){
    int result = 0;
    return result;
}

// end of file
