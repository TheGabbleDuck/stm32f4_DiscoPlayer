#ifndef __I2C_H
#define __I2C_H

// includes
#include "stm32f4xx.h"
#include "integer.h"

//	defines
//#define CSADDR_READ		0x95
//#define	CSADDR_WRITE	0x94

//	OR these for reads and writes
#define CS43_ADDR		0x94
#define CS43_READ		0x01
#define CS43_WRITE		0x00

#define CS43_RST_LO()	GPIOD->BSRRH = 0x0010;
#define CS43_RST_HI()	GPIOD->BSRRL = 0x0010;

//	Left in for completeness - deprecated.
/*
#define FLAG_MASK		((uint32_t)0x00FFFFFF)
#define	I2C_EVENT_MASTER_MODE_SELECT	((uint32_t)0x00030001)
// Master RECEIVER mode ----------------------------
// --EV7
#define  I2C_EVENT_MASTER_BYTE_RECEIVED	((uint32_t)0x00030040)  // BUSY, MSL and RXNE flags

// Master TRANSMITTER mode --------------------------
// --EV8
#define I2C_EVENT_MASTER_BYTE_TRANSMITTING	((uint32_t)0x00070080) // TRA, BUSY, MSL, TXE flags
// --EV8_2
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED	((uint32_t)0x00070084)  // TRA, BUSY, MSL, TXE and BTF flags
*/

// prototypes
void initI2C(I2C_TypeDef* port);
BYTE i2cReadSingle(I2C_TypeDef* port, BYTE addr);
void i2cWriteSingle(I2C_TypeDef* port, BYTE addr, BYTE data);
unsigned int I2C_CheckEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);
#endif
