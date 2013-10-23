
//	I2S low level functions for sending audio data.

//	includes
#include "i2s.h"
#include "stm32f4xx.h"
#include "hardware_Init.h"

//	defines

//	data + structs

//	main functions


//	Init I2S bus @ defaults, params in later
void initI2S(void){
	//	init for 44.1KHz, 16bit

	SPI3->CR1 		= 0x0000;			//	I2S / SPI off.

	SPI3->I2SPR		= 0x0206;			//	clk divisor and enable; mclk enabled.
	SPI3->CR2		= 0x0002;			//	TX DMA enable.
	SPI3->I2SCFGR	= 0x0E00;			//	i2s mode selected & enabled, mstr tx,
											//	pcm sync short frame, 16bit data, 16bpch
}

//	End of file
