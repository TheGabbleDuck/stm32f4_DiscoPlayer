#ifndef __HARDWARE_INIT
#define __HARDWARE_INIT

//	includes
#include "stm32f4xx.h"

//	defines
#define SD_SPI		SPI1	//	SD card
#define CS_I2S 		SPI3	//	I2S3
//#define I2S3 		SPI3	//	I2S audio port
#define DMA_Audio	DMA1_Stream7

//macros
//	PTA2 = _CS line for SD
#define SDCSLO() (GPIOA->BSRRH = 0x0004)	//	Set _CS low
#define SDCSHI() (GPIOA->BSRRL = 0x0004)	//	Set _CS High

//  Board setup function
void stm32f4discoInit(void);

#endif
