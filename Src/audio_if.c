//	Audio interface - sending and configuring codec

#include "stm32f4xx.h"
#include "audio_if.h"
#include "main.h"

//	defines

//	Functions

//	Reset audio codec
//	GPIOD4 low for a delay then high
void codecReset(void){
	CS43_RST_LO();
	delayLoop(1000000);	//	1M should give roughly 5ms delay
	CS43_RST_HI();
	delayLoop(1000000);	//	Delay again for clean pwr up
}

//	switch on codec to defaults
void codecPwrUp(void){
	unsigned char regData = 0;

	//	required initialisation settings
	//	page 32 CS43L22 datasheet
	i2cWriteSingle(I2C1, 0x02, 0x9E);		//	switch on
	delayLoop(1000000);						//	Small delay
	i2cWriteSingle(I2C1, 0x00, 0x99);		//
	i2cWriteSingle(I2C1, 0x47, 0x80);		//
	regData = i2cReadSingle(I2C1, 0x32);	//	read 0x32
	regData |= 0x80;						//	bit7 to 1
	i2cWriteSingle(I2C1, 0x32, regData);	//	write...
	regData &= 0x7F;						//	bit7 to 0
	i2cWriteSingle(I2C1, 0x32, regData);	//	write...
	i2cWriteSingle(I2C1, 0x00, 0x00);		//

	//	anything extra
	i2cWriteSingle(I2C1, 0x04, 0xAF);		//	HP always on, spkr always off

	//	slightly quieter with this
	//i2cWriteSingle(I2C1, 0x20, 0xF0);		//	master a volume at -6dB
	//i2cWriteSingle(I2C1, 0x21, 0xF0);		//	master b volume at -6dB

	//i2cWriteSingle(I2C1, 0x14, 0x10);		//	analog gain to +n dB
	//i2cWriteSingle(I2C1, 0x15, 0x10);		//	analog gain to +n dB

	regData = i2cReadSingle(I2C1, 0x08);	//	passthrough a
	regData &= 0xF0;						//	no analog inputs
	i2cWriteSingle(I2C1, 0x08, regData);	//	write back

	regData = i2cReadSingle(I2C1, 0x09);	//	passthrough b
	regData &= 0xF0;						//	no analog inputs
	i2cWriteSingle(I2C1, 0x09, regData);	//	write back

	//	configure for 16bit data to SDIN
	regData = i2cReadSingle(I2C1, 0x06);	//
	regData &= 0xE0;						//	clr relevant bits

	regData |= 0x10;						//	DSP mode on
	regData |= 0x04;						//	DACDIF I2S up to 24bit data
	regData |= 0x03;						//	AWL = 16bit data

	i2cWriteSingle(I2C1, 0x06, regData);	//	write back

	//	no difference below
	//regData = i2cReadSingle(I2C1, 0x05);	//	clocking
	//regData |= 0x01;						//	mclk div 2
	//i2cWriteSingle(I2C1, 0x05, regData);	//	write back

	//	headphone volume
	i2cWriteSingle(I2C1, 0x22, 0xE0);		//
	i2cWriteSingle(I2C1, 0x23, 0xE0);		//	headphone volume

	//	bass and treble
	//	1000b = 0dB gain on treble and bass. [treb][bass]7..0
	i2cWriteSingle(I2C1, 0x1F, 0x56);		//	+4.5treb, +3bass

	//i2cWriteSingle(I2C1, 0x1A, 0x18);		//
	//i2cWriteSingle(I2C1, 0x1B, 0x18);		//	18 = pcmx gain @ 12dB

}

//	make a beep specified by params.
//	Pre-reqs: make sure headphones are on etc
void codecBeep(unsigned int Freq, unsigned int length){
	i2cWriteSingle(I2C1, 0x1C, 0x71);	//	0x1C: beep freq and on time = 1KHz, 480ms
										//	0x1D left at default, 1.2s off, -6dB vol
	i2cWriteSingle(I2C1, 0x1E, 0x40);	//	One beep, tone at defaults
}

void codecAudioBusInit(void){
	initI2S();
}

void codecControlBusInit(void){
	initI2C(I2C1);
}

//	End of file
