//
#include "sdcard.h"
#include "integer.h"
#include "spi_sdcard.h"
#include "diskio.h"

//  Prototypes
/*
DRESULT MMC_disk_initialize(void);
DRESULT MMC_disk_status(void);  //  get status
DRESULT MMC_disk_read(void *buff, DWORD sector, BYTE count);
DRESULT MMC_disk_write(void *buff, DWORD sector, BYTE count);
DRESULT MMC_disk_ioctl(BYTE cmd, void *buff);
*/
//  Functions:

DRESULT MMC_disk_initialize(){
	static unsigned int i;
	static unsigned char responseArray[5];
	DRESULT result = RES_OK;

	while(responseArray[0] != 0x01){
		sdCommand(0, 0, &responseArray[0]);	//	cmd 0 expecting R1 response
	}

	//	Turn off crc
	sdCommand(59, 0, &responseArray[0]);

	//	Now send CMD8 check voltage
	sdCommand(8, 0x000001A5, &responseArray[0]);

	responseArray[0] = 0x55;	//	Set to non zero

	//	Loop round with acmd41 until card is reset
	//	responseArray[0] will = 0.
	for(i=0; responseArray[0] != 0; i++){
		sdCommand(55, 0, &responseArray[0]);
		sdCommand(41, 0x40000000, &responseArray[0]);
	}

	//	Now set SD card to high speed.
	//	10MHzish limit due to schonky wiring
	setSpiSpeed(0x08);	//	SPI1 = 84 / 8 = 10.5MHz

	asm("nop");

    return result;
}

//  Get status
DRESULT MMC_disk_status(){
    DRESULT result = RES_OK;
    return result;
}

//	Read <count> sectors starting from <sector> into
//	the buffer specified by *buff.
DRESULT MMC_disk_read(void* buff, DWORD sector, BYTE blockCount){
    DRESULT result = RES_OK;
    //	More than one sector?
    if(blockCount > 1){
		result = sdReadMulti(buff, sector, blockCount);
    }
    else
    {
    	result = sdReadSingle((unsigned char*)buff, sector);
    }
    return result;
}

DRESULT MMC_disk_write(void* buff, DWORD sector, BYTE blockCount){
    DRESULT result = RES_OK;
    return result;
}

DRESULT MMC_disk_ioctl(BYTE cmd, void* buff){
    DRESULT result = RES_OK;
    return result;
}

