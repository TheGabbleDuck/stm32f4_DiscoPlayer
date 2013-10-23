#ifndef __I2S_H
#define __I2S_H
//	Low level I2S bus access

//	includes
#include "stm32f4xx.h"

//	defines
/* SPI registers Masks */
#define CR1_CLEAR_MASK            ((uint16_t)0x3040)
#define I2SCFGR_CLEAR_MASK        ((uint16_t)0xF040)

/* RCC PLLs masks */
#define PLLCFGR_PPLR_MASK         ((uint32_t)0x70000000)
#define PLLCFGR_PPLN_MASK         ((uint32_t)0x00007FC0)

#define SPI_CR2_FRF               ((uint16_t)0x0010)
#define SPI_SR_TIFRFE             ((uint16_t)0x0100)

//	data
/**
  * @brief  I2S Init structure definition
  */

typedef struct
{

  uint16_t I2S_Mode;         /*!< Specifies the I2S operating mode.
                                  This parameter can be a value of @ref I2S_Mode */

  uint16_t I2S_Standard;     /*!< Specifies the standard used for the I2S communication.
                                  This parameter can be a value of @ref I2S_Standard */

  uint16_t I2S_DataFormat;   /*!< Specifies the data format for the I2S communication.
                                  This parameter can be a value of @ref I2S_Data_Format */

  uint16_t I2S_MCLKOutput;   /*!< Specifies whether the I2S MCLK output is enabled or not.
                                  This parameter can be a value of @ref I2S_MCLK_Output */

  uint32_t I2S_AudioFreq;    /*!< Specifies the frequency selected for the I2S communication.
                                  This parameter can be a value of @ref I2S_Audio_Frequency */

  uint16_t I2S_CPOL;         /*!< Specifies the idle state of the I2S clock.
                                  This parameter can be a value of @ref I2S_Clock_Polarity */
}I2S_InitTypeDef;

//	prototypes
void initI2S(void);
//void I2S_Init(SPI_TypeDef* SPIx, I2S_InitTypeDef* I2S_InitStruct);

#endif
