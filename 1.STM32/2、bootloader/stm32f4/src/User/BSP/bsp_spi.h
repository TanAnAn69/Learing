/*************************************************************************
 * File     : bsp_spi.h
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-10      aaron           first version
 ************************************************************************/
#ifndef __BSP_SPI_H
#define __BSP_SPI_H
#include "stdint.h"
#include "stm32f4xx.h"
#include "rtthread.h"
#include "stm32f4xx_spi.h"


extern uint16_t SPI1_SendData(SPI_InitTypeDef *iSPI1InitStruct,void (*iNssCmd)(FunctionalState NewState),uint16_t iSendData);
extern uint16_t SPI2_SendData(SPI_InitTypeDef *iSPI2InitStruct,void (*iNssCmd)(FunctionalState NewState),uint16_t iSendData);

#endif  // __BSP_SPI_H
