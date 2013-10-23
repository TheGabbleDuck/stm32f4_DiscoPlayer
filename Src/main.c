/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/

#include "stm32f4xx.h"		//	mcu header
#include "main.h"
#include "hardware_Init.h"  //  board support + init
#include "integer.h"		//	types declaration for fatfs
#include "diskio.h"			//	Low level stuff. Shouldn't need.
#include "ff.h"				//	fatfs header
#include "audio_if.h"		//	Codec interface.
#include "stdbool.h"		//

/*********************************************************************/
//	low level test stuff that shouldn't be here
//#include "spi_sdcard.h"
//#include "i2c.h"
//#include "stdio.h"

//	ping pong buffering & vars.
//	as read out gets to half way, read
//	another 8KB into alternate buffer
//	and switch over when ready.

/*
BYTE buffer1[8200];	//	8K test buffer
BYTE buffer2[8200];	//	8K test buffer
BYTE *bufferReadPtr = &buffer1[0];	//	init ptr
BYTE *bufferWritePtr = &buffer1[0];	//	init ptr
volatile unsigned int uiBufferSwitch = 1;	//
volatile unsigned int currentBuffer = 1;
volatile bool bReadMoreData = false;//	test when flagged.
volatile bool bSwitchTo2 = false;
volatile bool bSwitchTo1 = false;
volatile bool bReadingCard = false;
*/

//	1st/2nd half strategy.
//	16384B = 8192Words = 4096 samples == 92.8ms total == 46ms each half
BYTE audioBuffer1[16384];
BYTE audioBuffer2[16384];
BYTE headerBuffer[512];
//BYTE *audioRdPtr = &audioBuffer[0];
//BYTE *audioWrtPtr = &audioBuffer[0];
volatile bool bWriteBuffer1 = false;
//volatile bool b1stHalfDone = false;
volatile bool bWriteBuffer2 = false;
//volatile bool b2ndHalfDone = false;
volatile bool audioPlaying = false;
//unsigned int buffData = 0;			//	Data to read to spi buffers.


//tCS43L22InitStruct csInitStruct;

//	Button vars.
bool handleButton = false;
unsigned int debounceCounter = 0;
bool handlePress = false;

bool bAudioInit = false;
unsigned int testData = 0;			//	I2S test data
unsigned int testIncrement = 750;	//


//	FatFS objects
static FATFS FatFS;
static FIL file;
FRESULT fr;

/*********************************************************************/
//	ISRs

//	Handle incoming data
void USART2_IRQHandler(void)
{
    static unsigned char ucData = 0;

    asm("nop"); //  dbg breakpoint

    while(USART2->SR & 0x20)
    {
        ucData = USART2->DR;
    }
    USART2->DR = ucData;    //  echo char back
}

void EXTI0_IRQHandler(void)
{
    //	clear int
    EXTI->PR	= 0x00000001;	//	Clear int line 0 rc_w1
    handleButton = true;		//	flag to handle
    debounceCounter = 0;		//	reset every interrupt
    //	to aid debouncing
}

//	Audio freq interrupt
void TIM2_IRQHandler(void)
{

    TIM2->SR = ~TIM_SR_UIF;		//	Clear interrupt
}

//	1ms "tick" timer
void TIM7_IRQHandler(void)
{
	GPIOD->ODR ^= 0x8000;		//	toggle blue led
    //	clear int
    TIM7->SR = 0x0000;			//	clear interrupt rc_w0
    if(handleButton == true)
    {
        debounceCounter++;		//	Count of ms pressed
    }

    //	no int level handle once debounce is
    //	removed.
    if(debounceCounter > 50) 	//	Over N ms?
    {
        handleButton = false;	//	handled, stop counting
        debounceCounter = 0;	//	reset
        handlePress = true;		//	Now do a button press thing
    }
}

void DMA1_Stream6_IRQHandler(void)
{
    asm("nop");
}

void DMA1_Stream7_IRQHandler(void)
{
    asm("nop");
    if(DMA1->HISR & 0x08000000) 	//	xfer complete
    {
        asm("nop");
        GPIOD->ODR ^= 0x1000;		//	led toggle
    }
    if(DMA1->HISR & 0x04000000) 	//	Half xfer - trigger alternate buffer write
    {
        asm("nop");
        if(DMA_Audio->CR & 0x00080000) 	//	CT = 1? Write to buffer 1
        {
            bWriteBuffer1 = true;
        }
        else 								//	CT = 0. Write to buffer 2
        {
            bWriteBuffer2 = true;
        }
    }

    //	clear down flags
    DMA1->HIFCR = 0x0F400000;			//	clr pending
}

/*********************************************************************/
//	Main functions
void delayLoop(volatile unsigned int vuiLoops)
{
    while(vuiLoops--) {};
}

void writeUart(unsigned char* buffer, unsigned int numChars)
{
    //	write n chars to USART2.
    while(0 < (numChars--))
    {
        USART2->DR = *(buffer++);
        while(!(USART2->SR & USART_SR_TC)) {};	//	wait for TX complete
    }
}

int main(void)
{
    //unsigned int uiInt = 0;
    //unsigned char csPwr1 = 0;			//	Power control
    //unsigned char csPwr2 = 0;			//
    unsigned char asciiData = 0x61;
    BYTE diskStatus = 0;
    unsigned int byteCounter = 0;


    //	Setup hardware - LED indication for staged
    stm32f4discoInit();					//	LEDs enabled
    GPIOD->ODR |= 0x1000;				//	green LED on
    codecReset();						//	audio codec reset
    GPIOD->ODR |= 0x2000;				//	amber LED on
    codecControlBusInit();				//	i2c initialisation
    GPIOD->ODR |= 0x4000;				//	red LED on
    codecAudioBusInit();				//	I2C and I2S buses set up
    GPIOD->ODR |= 0x8000;				//	blue LED on
    codecPwrUp();						//	init and switch on

    GPIOD->ODR &= 0x0FFF;				//	all off
    //bAudioInit = true;					//	flag as booted

    //	fill audiobuffer with lower case ascii.
    for(byteCounter = 0; byteCounter < 16384; byteCounter++)
    {
        audioBuffer1[byteCounter] = asciiData++;
        if(asciiData > 0x7A)
        {
            asciiData = 0x61;
        }
    }

    //	DMA setup
    DMA_Audio->CR &= 0xFFFFFFFE;				//	clear bit
    while (DMA_Audio->CR & 0x1) {};				//	wait for EN = 0
    DMA_Audio->PAR = (unsigned int)(&(SPI3->DR));		//	destination
    DMA_Audio->M0AR = (unsigned int)&audioBuffer1[0];	//	source
    DMA_Audio->M1AR = (unsigned int)&audioBuffer2[0];	//	source
    DMA_Audio->NDTR = 8192;					//	items to xfer

    //	set up flow control, ints etc
    DMA_Audio->CR = 0
                    |0b0000<<28				//	***RSVD***
                    |0b000<<25				//	CHSEL		=	0 = SPI3TX
                    |0b00<<23				//	MBURST		=	00 = single xfer
                    |0b00<<21				//	PBURST		=	00 = single xfer
                    |0b0<<20				//	***RSVD***
                    |0b0<<19				//	CT			=	0 = Target = M0 (in double buffer)
                    |0b1<<18				//	DBM or RSVD	=	0 = No buffer switch at xfer end
                    |0b01<<16				//	PL			=	01 = medium priority level
                    |0b0<<15				//	PINCOS		=	0 = periph inc based on PSIZE
                    |0b01<<13				//	MSIZE		=	00 = Byte, 01 = 16bit 10 = 32bit
                    |0b01<<11				//	PSIZE		=	as above
                    |0b1<<10				//	MINC		=	1 = mem ptr increments
                    |0b0<<9					//	PINC		=	0 = periph ptr fixed
                    |0b1<<8					//	CIRC		=	0 = circular mode disabled
                    |0b01<<6				//	DIR			=	01 = mem to periph
                    |0b0<<5					//	PFCTRL		=	0 = DMA is flow ctrlr
                    |0b1<<4					//	TCIE		=	x = xfer complete ints
                    |0b1<<3					//	HTIE		=	x = half xfer ints
                    |0b0<<2					//	TEIE		=	0 = xfer error
                    |0b0<<1;				//	DMEIE		=	0 = direct mode error


    //	old DMA cr value
    //0b0x08010448;									//	CHSEL = 4 (USART2)
    //	ints halfway
    //	m -> p, bytesize
    //	enable in button press
    //DMA1_Stream6->CR |= 0x1;						//	enable channel

    //	Test read of text file in SD card root.

    diskStatus = disk_initialize(0);	//	Init MMC

    fr = f_mount(0, &FatFS);			//	mount disk as 0:

    if(fr != 0)
    {
        asm("nop");
    }
    else
    {
        asm("nop");
        fr = f_chdir("0:/Wav");
        fr = f_open(&file, "switch.wav", FA_OPEN_EXISTING | FA_READ);
        fr = f_read(&file, &headerBuffer, 512, &byteCounter);
        if(fr == 0)
        {
            fr = f_lseek(&file, 0x2E);			//	set to data beginning.
            fr = f_read(&file, &audioBuffer1, 16384, &byteCounter);
            //audioRdPtr += 0x2E;				//	Offset of wave data beginning.(for this particular file)
            //uiBufferSwitch = 1;				//	pointing to buffer 1.
            //	last thing. Trigger play in 44100Hz ints.
            //bAudioInit = true;				//	flag as booted
        }
    }

    //	test word
    //I2S3ext->DR = 0xF00F;
    //while(!(I2S3ext->SR & 0x02)){};

    while(1)
    {
        //	LED flashes for debug visual indication only. Main
        //	state machine will handle audio playback / events
        //	when in place.
        //GPIOD->BSRRL = 0x8000;				//  Set bit 15
        //delayLoop(1000000);
        //GPIOD->BSRRH = 0x8000;				//  clr bit 15
        //delayLoop(1000000);

        //GPIOD->ODR ^= 0x8000;					//	toggle blue LED

        //USART2->DR = asciiData++;				//	test transmit
        //if(asciiData > 0x7A){
        //	asciiData = 0x61;
        //}

        if(bWriteBuffer1 == true)
        {
            GPIOD->BSRRL = 0x2000;				//  orange
            fr = f_read(&file, &audioBuffer1[0], 16384, &byteCounter);
            GPIOD->BSRRH = 0x2000;				//  orange
            bWriteBuffer1 = false;
        }

        if(bWriteBuffer2 == true)
        {
            GPIOD->BSRRL = 0x4000;				//  orange
            fr = f_read(&file, &audioBuffer2[0], 16384, &byteCounter);
            GPIOD->BSRRH = 0x4000;				//  orange
            bWriteBuffer2 = false;
        }

        /*
        I2S3ext->DR = 0x0F0F;
        while(!(I2S3ext->SR & 0x02)){};

        I2S3ext->DR = 0x0C0C;
        while(!(I2S3ext->SR & 0x02)){};
        */

        //	Button press to handle?
        if(handlePress == true)
        {
            handlePress = false;				//

            if(audioPlaying == false){			//	Not played yet?
				DMA_Audio->CR |= 0x1;			//	enable channel
				fr = f_lseek(&file, 0x2E);		//	set to data beginning.
				fr = f_read(&file, &audioBuffer1, 16384, &byteCounter);
            }
            else{
				audioPlaying = false;
				DMA_Audio->CR &= 0xFFFFFFFE;	//	disable DMA.
            }
        }


    }
}
