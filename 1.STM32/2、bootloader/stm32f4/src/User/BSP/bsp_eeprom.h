/*************************************************************************
 * File     : bsp_eeprom.h
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-13      aaron           first version
 ************************************************************************/
#ifndef __BSP_EEPROM_H
#define __BSP_EEPROM_H
#include "stdint.h"

extern void EEPROM_Init(void);
extern void EEPROM_Wr8(uint16_t iAddr,uint8_t iData);
extern void EEPROM_Wr16(uint16_t iAddr,uint16_t iData);
extern void EEPROM_Wr32(uint16_t iAddr,uint32_t iData);
extern void EEPROM_Wrf(uint16_t iAddr,float iData);
extern void EEPROM_WrStr(uint16_t iAddr,const char *iData);

extern uint8_t EEPROM_Rd8(uint16_t iAddr);  
extern uint16_t EEPROM_Rd16(uint16_t iAddr);  
extern uint32_t EEPROM_Rd32(uint16_t iAddr);  
extern float EEPROM_Rdf(uint16_t iAddr);  
extern void EEPROM_RdStr(uint16_t iAddr,char *iBuf,uint16_t iStrLen,uint16_t iBufLen);		  

extern void EEPROM_Read(uint16_t iRegAddr, uint8_t *iBuf, uint16_t iDataLen,uint16_t iBufLen);
extern void EEPROM_Write(uint16_t iRegAddr, uint8_t *iData, uint16_t iLen);

#endif  // __BSP_EEPROM_H
