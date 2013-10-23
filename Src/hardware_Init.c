
// Hardware initialisation

//  Includes
#include "stm32f4xx.h"
#include "hardware_Init.h"

//  Board defines
#define SD_USE_SDIO 0		//  0=SPI
							//  1=SDIO

//  Variables

//  Functions

//  initBoard
//  set up hardware and low level IO for any GPIO
//  and peripherals
void stm32f4discoInit(void)
{
	//unsigned short tmpReg = 0;
	//unsigned int i = 0;
    //  Global clock enables

    //	Pre-scalers:
    //	AHB1 = 168MHz
    //	APB1 = 42MHz
    //	APB2 = 84MHz

	/*
		CLOCK GATING SETUP
	*/

	//
    RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN		//	DMA xfers
					|RCC_AHB1ENR_GPIOAEN	//
					|RCC_AHB1ENR_GPIOBEN	//
					|RCC_AHB1ENR_GPIOCEN	//
					|RCC_AHB1ENR_GPIODEN);	//  GPIOA,B,C,D clk enables
	//	42MHz
    RCC->APB1ENR |= (RCC_APB1ENR_SPI3EN		//	SPI3 for I2S3 audio
					|RCC_APB1ENR_USART2EN	//	USART2 for debug
					//|RCC_APB1ENR_TIM2EN		//	timer 2 for audio timing
					|RCC_APB1ENR_TIM7EN		//	basic timer for low priority h/w ints
					|RCC_APB1ENR_I2C1EN);	//	I2C for CS43L22 DAC control 0x94

	//	84MHz
    RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN		//  SPI1 (SD card)
					|RCC_APB2ENR_SYSCFGEN);	//	System config incl exti
											//	NB USART1EN taken out - not used?

	//	Debug code - checking start up
//	GPIOD->MODER |= 0x55000000;				//
//	GPIOD->OSPEEDR |= 0x55000000;			//
//	asm("nop");
//	GPIOD->ODR	|= 0xF000;					//	All leds on
//	for(i=0;i<100000;i++){
//		asm("nop");
//	}
//	GPIOD->ODR	&= 0x0FFF;					//	All leds off

#if (SD_USE_SDIO == 0)						//  Set up for SPI

	//	portA2 as GPIO, A5,6,7 as AF5 for SPI1
	GPIOA->MODER	&= 0xFFFF03CF;			//	clr 2,5,6 & 7
	GPIOA->MODER	|= 0x0000A810;			//	A5,6,7 as AF5, A2 as GPIO OP
	GPIOA->OSPEEDR	|= 0x0000A820;			//	fast IO on pins inc _CS
	GPIOA->AFR[0]	|= 0x55500000;			//	5,6,7 @ AF5 == SPI1

	//	SPI1 peripheral - off APB2 @ 84MHz	//	divisors of 2,4,8,16,32,64,128 & 256 (000:111)
	SD_SPI->CR1		|= 0x0374;				//	SPI1 enabled @ lowest speed, 328KHz.

	SDCSHI();								//  Set _CS to 1

#else										//  set up for SDIO
	//  SDIO clock enable
    RCC->APB2ENR	|= RCC_APB2ENR_SDIOEN;	//	Enable SDIO peripheral clk

    //	SDIO pins setup
    GPIOD->AFR[0]	|= 0x00000C00;			//  C00h = PD2 as SDIO cmd pin
	GPIOC->MODER	|= 0x02AA0000;			//  Pins 8,9,10,11,12 as alternate func
	GPIOC->OSPEEDR	|= 0x02AA0000;			//  Fast (50MHz) on above
	GPIOC->AFR[1]	|= 0x000CCCCC;			//  AF12 on PC8,9,10,11,12

#endif
	/*
		PINS SETUP
	*/

	//	User button - input on PA0
	GPIOA->MODER	&= 0xFFFFFFFC;			//	clr PA0 function = GPIO input
	GPIOA->AFR[0]	&= 0xFFFFFFF0;			//	GPIO on PA0, AF6 on PA4
	GPIOA->MODER	|= 0x00000200;			//	AF on PA4
	GPIOA->AFR[0]	|= 0x00060000;			//	AF6

	//	PB6+9 = I2C port SCL and SDA
	//	I2C1 pins setup
	GPIOB->MODER	&= 0xFFF3CFFF;			//	clr 6 and 9
	GPIOB->MODER	|= 0x00082000;			//	B6 & B9 = AF
	GPIOB->OSPEEDR	|= 0x00082000;			//	50MHz limit
	GPIOB->OTYPER	|= 0x00000240;			//	Open drain on 6 & 9
	GPIOB->PUPDR	|= 0x00041000;			//	Pull-up on 6+9
	GPIOB->AFR[0]	|= 0x04000000;			//	B6 = AF4 == I2C1 SCL
	GPIOB->AFR[1]	|= 0x00000040;			//	B9 = AF4 == I2C1 SDA


	//	SPI3 pins setup
	GPIOC->MODER	&= 0xFC0F3FFF;			//	Clr ptc7,10,11,12
	GPIOC->MODER	|= 0x02A08000;			//	PTC7,10,11,12 = alt function
	GPIOC->OSPEEDR	|= 0x02A08000;			//	50MHz speed on ptc10,11,12
	GPIOC->AFR[0]	|= 0x60000000;			//	C7 = AF6 = I2S3 MCK
	GPIOC->AFR[1]	|= 0x00066600;			//	10,11,12 = AF6 = SPI3/I2S3

	//	PORTD config - LED pins & usart2 & D4=CS reset
	GPIOD->MODER    |= 0x55000120;			//  output on 12,13,14,15, alt func on PD2
	GPIOD->OSPEEDR  |= 0xAA000220;			//  fast(50Mhz) output on LED pins and mosi

	//TIM2->DIER 		= 0x0001;				//	enable ints
	//TIM2->PSC		= 7;					//	prescaler for 42MHz apb1 clk
	//TIM2->ARR		= 136;					//	42M / 44100 = 952 == 34*28 == 136*7
	//TIM2->CR1		= 0x0005;				//	enabled, multi shot

	//	Timer 7 1KHz tick - low priority int / will handle jitter.
	TIM7->DIER 		= 0x0001;				//	enable ints
	TIM7->PSC		= 0x0001;				//	prescaler for 42MHz apb1 clk +1
	TIM7->ARR		= 21000;				//	42M / 21000 = 1000 with PSC = [1]+1
	TIM7->CR1		= 0x0005;				//	enabled, multi shot

	//	USART1 pins
	//GPIOB->MODER	|= 0x0000A000;			//  AF on 6+7
	//GPIOB->OSPEEDR	|= 0x0000A000;			//  fast(50MHz) on 6+7
	//GPIOB->AFR[0]	|= 0x77000000;			//  AF7 (usart1) on PB6+7

	//	USART2 pins (usart1 & I2C1 for CS43L22 clash)
	GPIOD->MODER	|= 0x00002800;			//	AF on 5&6
	GPIOD->OSPEEDR	|= 0x00002800;			//	50MHz op speed
	GPIOD->AFR[0]	|= 0x07700000;			//	AF7 on 5&6

	//	PA0 ext interrupt config.
	SYSCFG->EXTICR[0] &= 0x00000000;		//	Already 0x0 @ rst, = PA0 source
	EXTI->IMR		|= 0x00000001;			//	int from line 0 not masked.
	EXTI->FTSR		|= 0x00000001;			//	Falling edge ints. (button release)

	/*
		PERIPHERAL SETUP
	*/

	//	I2C1 peripheral
	I2C1->CR1 		= 0x8408;				//	Standard i2c setup, peripheral
	I2C1->CR2		= 0x002A;				//	*disabled* in i2c mode with no ints
	I2C1->CCR		= 0x80D2;				//	100KHz divisor from 42MHz
	I2C1->TRISE		= 0x0029;				//	1000ns / (1/42M)
	I2C1->CR1		|=0x0001;				//	enabled


	//	USART peripheral
	USART2->BRR		= (45<<4) | 9;			//  9/16 = 0.5625 == 45.5625
	USART2->CR1		= 0x002C;				//  |= 0x2000 to enable, Int on RX, 8d,np,1sb
	USART2->CR2		= 0x0000;				//  Lin disable, 1sb, CPOL = 0
	USART2->CR3		= 0x0080;				//  DMA tx enbl, rts, cts and stuff disabled
	USART2->GTPR	= 0x0101;				//  Minimum guard time & prescaler values
	USART2->CR1		|= 0x2000;				//  Enable USART1

	//	Setup for I2S3 using SPI3 bus
	//	Standard I2S protocol (not left/right justified)



    //	now enable ints
	NVIC_EnableIRQ(USART2_IRQn);			//  Enable USART1 irqs
	NVIC_EnableIRQ(EXTI0_IRQn);				//	PA0 IRQ for user button.
	//NVIC_EnableIRQ(TIM2_IRQn);				//	Audio freq ints
	NVIC_EnableIRQ(TIM7_IRQn);				//	1ms ints
	NVIC_EnableIRQ(DMA1_Stream7_IRQn);		//	Audio DMA buffer half ints


}


//  End of hardware init
