


#ifndef __SPI_SDCARD_H
#define __SPI_SDCARD_H

//	includes
#include "stm32f4xx.h"
#include "diskio.h"
#include "hardware_init.h"

//	Board defines


//	Data
unsigned char *cmdPtr;

typedef struct{
	union{
		struct{
			unsigned char Header;	//	1 byte header
			unsigned int Payload;	//	4 byte payload
			unsigned char Trailer;	//	1 byte crc
		}cmdStruct;
		unsigned char cmdArray[6];
	};
}tCommandStruct;

//	Prototypes

//	Set the SPI speed to n MHz.
unsigned int setSpiSpeed(unsigned int speed);

//	Delay of n loops
void delayLoop(volatile unsigned int uiLoops);

//	CRC7 of sd card command array
void sd_crc7(unsigned char byteArray[]);

//	populate response with the command response...
void sdCommand(unsigned int cmd, unsigned int payload, unsigned char *response);

DRESULT sdReadSingle(unsigned char* buffer, unsigned int sector);

DRESULT sdReadMulti(unsigned char* buffer, unsigned int sector, unsigned int blockCount);

DRESULT sdWriteSingle(unsigned char* buffer, unsigned int sector);

DRESULT sdWriteMulti(unsigned char* buffer, unsigned int sector, unsigned int blockCount);

#endif
