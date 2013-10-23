

//	Defines

//	Includes
#include "stm32f4xx.h"
#include "spi_sdcard.h"
#include "main.h"		//	for delayLoop() function

//	Data
//unsigned char cmdArray[6];
tCommandStruct sdCmd;
unsigned char *cmdPtr = &sdCmd.cmdArray[0] ;

//	Prototypes
/*
unsigned int setSpiSpeed(unsigned int speed);
void delayLoop(volatile unsigned int uiLoops);
void sd_crc7(unsigned char byteArray[]);
void sdCommand(unsigned int cmd, unsigned int payload, unsigned char *response);
void sdReadSingle(void *buffer, unsigned int sector);
void sdReadMulti(void *buffer, unsigned int sector, unsigned int blockCount);
void sdWriteSingle(void *buffer, unsigned int sector);
void sdWriteMulti(void *buffer, unsigned int sector, unsigned int blockCount);
*/
//	Functions:::

//	SPI1 = APB2 = 84MHz.
unsigned int setSpiSpeed(unsigned int divisor){

	divisor |= 0x08;							//	spe on bit
	SD_SPI->CR1 &= 0xFF87;						//	Clear baud divisor and disable module
	SD_SPI->CR1 |= (divisor&0x0F)<<3;				//	re-enable module and set divisor

	if(SD_SPI->CR1 && SPI_CR1_SPE){
		return 0;								//	SPI enabled, ok
	}
	else{
		return 1;								//	not enabled. Badness.
	}
}

void sd_crc7(unsigned char byteArray[])			//	Software 7bit CRC check for cmdArray
{
  	unsigned char ibit, c, crc, *cmdPtr;		//
  	unsigned int i=0;							//

  	cmdPtr = &byteArray[0];						//	set pointer
  	crc = 0x00;									//	Set initial value
 	for (i = 0; i < 5; i++, cmdPtr++){
		c = *cmdPtr;
		for (ibit = 0; ibit < 8; ibit++){
			crc = crc << 1;
			if ((c ^ crc) & 0x80)
			crc = crc ^ 0x09;
			{	// ^ is XOR
				c = c << 1;
			}
		}
		crc = crc & 0x7F;
	}

	crc<<=1;									//	MMC card stores the result in the top 7 bits so shift them left 1                                                                   // Should shift in a 1 not a 0 as one of the cards I have won't work otherwise
	crc+=1;										//	shift left, put a 1 in
	byteArray[5] = crc;							//	append crc

}//	End of software crc7

//	Functions
void sdCommand(unsigned int cmd, unsigned int payload, unsigned char *response)
{
    volatile unsigned int i = 0;
    unsigned char dummyData = 0;

    cmdPtr = &sdCmd.cmdArray[0];				//

    sdCmd.cmdArray[0] = cmd|0x40;				//
    sdCmd.cmdArray[4] = payload & 0xFF;			//	LSB
    sdCmd.cmdArray[3] = payload>>8 & 0xFF;		//	...
    sdCmd.cmdArray[2] = payload>>16 & 0xFF;		//	...
    sdCmd.cmdArray[1] = payload>>24 & 0xFF;		//	MSB
    //cmdArray[5] is crc byte


    if(sdCmd.cmdArray[0] == 0x40) 				//	Card reset, looking for 0x00 response
    {
        sdCmd.cmdArray[5] = 0x95;				//	Put CRC in manually
        SDCSLO();
        delayLoop(50);
        //	Clock out 80 clocks
        SD_SPI->CR1 &= 0xFF87;					//	Clear baud divisor and disable module
        SD_SPI->CR1 |= 0x0070;					//	re-enable module and divisor set for /128 == 328.125KHz (<400KHz = ok)

        for(i=0; i<10; i++)
        {
            SD_SPI->DR = 0xFF;					//	clock out all ones...
            while(!(SD_SPI->SR & SPI_SR_RXNE)) {};//	Wait for data back
            dummyData = SD_SPI->DR;				//	Read data
        }
    }else{/*Not a cmd 0, proceed with usual command op */
    	sd_crc7(&sdCmd.cmdArray[0]);				//	or maybe calc crc
    	SDCSLO();
        delayLoop(50);							//	wait
        SD_SPI->DR = 0xFF;						//	Preamble byte...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		dummyData = SD_SPI->DR;					//	Read data
    }


    //	Now send command
    for(i=0; i<6; i++)
    {
        SD_SPI->DR = sdCmd.cmdArray[i];			//
        while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
        dummyData = SD_SPI->DR;
    }
	SD_SPI->DR = 0xFF;							//	clock out all ones...
	while(!(SD_SPI->SR & SPI_SR_RXNE)) {};		//	Wait for data back
	dummyData = SD_SPI->DR;						//	Read data - post-amble byte

	switch(cmd){
	case(0):									//	R1 response
	case(17):
	case(18):
	case(24):
	case(25):
	case(55):
	case(59):
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		*response = SD_SPI->DR;					//	Read data - reply byte

		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		dummyData = SD_SPI->DR;					//	Read data - trailer byte
		break;
	case(12):									//	R1b response stop trans
		while(0 != dummyData){
			SD_SPI->DR = 0xFF;							//	clock out all ones...
			while(!(SD_SPI->SR & SPI_SR_RXNE)) {};		//	Wait for data back
			dummyData = SD_SPI->DR;						//	Read data - post-amble byte
		}
		while(0 == dummyData){
			SD_SPI->DR = 0xFF;							//	clock out all ones...
			while(!(SD_SPI->SR & SPI_SR_RXNE)) {};		//	Wait for data back
			dummyData = SD_SPI->DR;						//	Read data - post-amble byte
		}

		break;
	case(13):									//	R2 response
		break;
	case(58):									//	R3 response
		break;
	case(8):									//	R7 response
		for(i=0;i<5;i++){
			SD_SPI->DR = 0xFF;						//	clock out all ones...
			while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
			*(response++) = SD_SPI->DR;				//	Read data into R7 response 5 bytes
		}
		break;

	default:
		break;
	}

	//	Here we want to keep CS low if it's a read / write operation.
	switch(cmd){
	case(17):									//	Don't de-select the card for reads and writes
	case(18):
	case(24):
	case(25):
		break;
	default:									//	Otherwise, deselct and go
		delayLoop(50);
		SDCSHI();
		asm("nop");								//	For breakpoint
		break;
	}

	if(dummyData == 0){							//	stop warning about unused dummyData
		asm("nop");
	}

}

DRESULT sdReadSingle(unsigned char* buffer, unsigned int sector){
	//	read one sector to *buffer.
	//static unsigned char cmdResponse;
	static unsigned char *responsePtr;
	unsigned char dummyData = 0x00;
	static unsigned int i = 0, intCRC = 0;

	sdCommand(17, sector, responsePtr);	//	command to Read a sector
	//	_CS is still low, wait until 0xFFs, then 0xFE
	while(dummyData != 0xFF){					//	Waiting for 0x00s to end
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		dummyData = SD_SPI->DR;					//	Read data
	}

	//	Last pre block token = 0xFE
	while(0xFF == dummyData){
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		dummyData = SD_SPI->DR;					//	Read data
	}
	//	Now read 512bytes + 2 bytes CRC (discard CRC)
	for(i=0;i<512;i++){
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		*(buffer++) = SD_SPI->DR;				//	Read data to buffer.
	}

	// now read CRC
	SD_SPI->DR = 0xFF;						//	clock out all ones...
	while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
	intCRC = SD_SPI->DR;					//	Read CRC

	SD_SPI->DR = 0xFF;						//	clock out all ones...
	while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
	intCRC<<=8;
	intCRC |= SD_SPI->DR;					//	Read CRC 2nd byte

	SD_SPI->DR = 0xFF;						//	Last trailing byte
	while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
	dummyData = SD_SPI->DR;					//	Read buffer

	delayLoop(50);
	SDCSHI();								//	deselect card
	asm("nop");								//	For breakpoint
	return RES_OK;
}

DRESULT sdReadMulti(unsigned char* buffer, unsigned int sector, unsigned int blockCount){
	//	read one sector to *buffer.
	//static unsigned char cmdResponse;
	static unsigned char *responsePtr;
	unsigned char dummyData = 0x00;
	static unsigned int i = 0, j = 0, intCRC = 0;

	//	start with CMD18, stop with CMD12.
	sdCommand(18, sector, responsePtr);	//	command to Read a sector

	//	_CS is still low, wait until 0xFFs, then 0xFE
	while(dummyData != 0xFF){					//	Waiting for 0x00s to end
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		dummyData = SD_SPI->DR;					//	Read data
	}

	//	Last pre block token = 0xFE
	while(0xFF == dummyData){
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		dummyData = SD_SPI->DR;					//	Read data - last/pre-block byte = 0xFE
	}
	//	now read n blocks of data
	for(i=0; i < blockCount; i++){				//	n blocks of data
		for(j=0;j<512;j++){
			SD_SPI->DR = 0xFF;						//	clock out all ones...
			while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
			*(buffer++) = SD_SPI->DR;				//	512 Bytes of data
		}
		//	Now read 2 CRC bytes
		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		intCRC = SD_SPI->DR;					//	Read crc

		SD_SPI->DR = 0xFF;						//	clock out all ones...
		while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
		intCRC<<=8;
		intCRC |= SD_SPI->DR;					//	Read crc

		//	Now read bytes until 0xFE
		dummyData = 0xFF;
		while(0xFE != dummyData){
			SD_SPI->DR = 0xFF;						//	clock out all ones...
			while(!(SD_SPI->SR & SPI_SR_RXNE)) {};	//	Wait for data back
			dummyData = SD_SPI->DR;					//	Read data
		}
		//	last byte was 0xFE - ready for another block.
	}

	sdCommand(12, 0x00000000, responsePtr);		//	Stop transmission

	//	End of read.
	delayLoop(50);
	SDCSHI();									//	deselect card
	asm("nop");									//	For breakpoint

	return RES_OK;
}

DRESULT sdWriteSingle(unsigned char* buffer, unsigned int sector){
	//	write one sector to *buffer.
	return RES_ERROR;
}

DRESULT sdWriteMulti(unsigned char* buffer, unsigned int sector, unsigned int blockCount){
	//	write multiple sectors
	return RES_ERROR;
}
//	End of file
