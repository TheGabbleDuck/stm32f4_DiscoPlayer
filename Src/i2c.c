//	i2c routines for reading / writing.

//	includes
#include "stm32f4xx.h"
#include "i2c.h"
#include "integer.h"
//	data

void initI2C(I2C_TypeDef* port){
	port->CR1 &= 0x7FFF;		//	Out of reset
	//port->CR1 |= 0x0001;		//	Peripheral enabled

	port->CR1 		= 0x0008;				//	Standard i2c setup, peripheral
	port->CR2		= 0x002A;				//	*disabled* in i2c mode with no ints
	port->CCR		= 0x80D2;				//	100KHz divisor from 42MHz
	port->TRISE		= 0x0029;				//	1000ns / (1/42M)
	port->CR1		|=0x0001;				//	enabled

	//port->CR1		|= I2C_CR1_STOP;		//	put a stop out to reset.
	//while(port->SR2 & I2C_SR2_BUSY){};		//
}

BYTE i2cReadSingle(I2C_TypeDef* port, BYTE addr){

	//	Generate start condition
	port->CR1 |= I2C_CR1_START;				//	generate start
	while(!(port->SR2 & I2C_SR2_MSL)){};
	asm("nop");

	//	Write the slave address(write) = R/_W
	port->DR = CS43_ADDR|CS43_WRITE;
	while(!(port->SR1 & 0x82)){};			//	TxE & ADDR bits high chk
	while(!(port->SR2 & 0x07)){};			//
	asm("nop");

	//	Write the map address
	port->DR = addr;
	while(!(port->SR1 & 0x82)){};			//	TxE & ADDR bits high chk
	while(!(port->SR2 & 0x07)){};			//
	asm("nop");

	//	Generate stop
	port->CR1 |= I2C_CR1_STOP;				//	generate stop
	while(port->SR2 & I2C_SR2_MSL){};		//	Wait for clear

	//	Generate start
	port->CR1 |= I2C_CR1_START;				//	generate start
	while(!(port->SR2 & I2C_SR2_MSL)){};

	//	Write the slave address(read)
	port->DR = CS43_ADDR|CS43_READ;			//
	while(!(port->SR1 & 0x82)){};			//	TxE & ADDR bits high chk
	while(!(port->SR2 & 0x07)){};			//
	asm("nop");

	//	NACK as last byte...
	port->CR1 &= ~I2C_CR1_ACK;				//	Enable acknowledgment
	while(!(port->SR1 & I2C_SR1_RXNE)){};	//
	asm("nop");								//


	//	Generate stop condition
	port->CR1 |= I2C_CR1_STOP;				//	generate stop
	while(port->SR2 & I2C_SR2_MSL){};		//	Wait for clear

	return port->DR;						//	Booyah.
}

void i2cWriteSingle(I2C_TypeDef* port, BYTE addr, BYTE data){
	//	Generate start condition
	port->CR1 |= I2C_CR1_START;				//	generate start
	while(!(port->SR2 & I2C_SR2_MSL)){};
	asm("nop");

	//	Write the slave address(write) = R/_W
	port->DR = CS43_ADDR|CS43_WRITE;
	while(!(port->SR1 & 0x82)){};			//	TxE & ADDR bits high chk
	while(!(port->SR2 & 0x07)){};			//
	asm("nop");

	//	Write the map address
	port->DR = addr;
	while(!(port->SR1 & 0x82)){};			//	TxE & ADDR bits high chk
	while(!(port->SR2 & 0x07)){};			//
	asm("nop");

	port->DR = data;
	while(!(port->SR1 & 0x82)){};			//	TxE & ADDR bits high chk
	while(!(port->SR2 & 0x07)){};			//
	asm("nop");

	//	Generate stop condition
	port->CR1 |= I2C_CR1_STOP;				//	generate stop
	while(port->SR2 & I2C_SR2_MSL){};		//	Wait for clear
}

/*
unsigned int I2C_CheckEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
  uint32_t lastevent = 0;
  uint32_t flag1 = 0, flag2 = 0;
  ErrorStatus status = ERROR;

  //Check the parameters
  //assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  //assert_param(IS_I2C_EVENT(I2C_EVENT));

  //Read the I2Cx status register
  flag1 = I2Cx->SR1;
  flag2 = I2Cx->SR2;
  flag2 = flag2 << 16;

  //Get the last event value from I2C status register
  lastevent = (flag1 | flag2) & FLAG_MASK;

  //Check whether the last event contains the I2C_EVENT
  if ((lastevent & I2C_EVENT) == I2C_EVENT)
  {
    //SUCCESS: last event is equal to I2C_EVENT
    status = 0;	//	SUCCESS
  }
  else
  {
    //ERROR: last event is different from I2C_EVENT
    status = 1;	//	ERROR
  }
  //Return status
  return status;
}
*/
//	End of file
