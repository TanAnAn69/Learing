/*************************************************************************
 * File     : bsp_at24cxx.c
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-13      aaron           first version
 ************************************************************************/
/*** Including ***/
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "rtthread.h"
#include "finsh.h"
#include "bsp_iic.h"
#include "bsp_eeprom.h"
#include "bsp_common.h"

FILE_VERSION_EXPORT(bsp_eeprom,V1.0_20190312)

static uint8_t DbgOptEeprom=0;
DBG_EXPORT(DbgOptEeprom,eeprom,eeprom debug option)
#define EEPROM_DEBUG(opt,format,...) if(opt&DbgOptEeprom) {rt_kprintf("[eeprom:%02x]%s(%d): "format"",opt,__FUNCTION__,__LINE__,##__VA_ARGS__);}


static sI2CM_CfgInfo EEP_I2CMInfo;

#define EE_TYPE AT24C64
#define EE_DEV_ADDR 0xA0
#define ADDRESS_MAX (64*1024/8)

typedef enum{
    AT24C02 = 0x00,
    AT24C08 = 0x01,
    AT24C16 = 0x02,
    AT24C32 = 0x03,
    AT24C64 = 0x04
}EETypeDef;


static void EEPROM_Delayus(uint16_t n)
{
	uint16_t i,j;
    uint32_t k = 0;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < 27; j++)
		{
			k++;
		}
	}
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  */
static uint8_t EEPROM_ReadByte(uint16_t iRegAddr)
{
	uint16_t i=0;
	uint8_t data = 0;
    uint8_t devaddr;
    uint16_t regaddr;
    eI2CM_Error err;
    
    if(EE_TYPE > AT24C16) 
    {
        devaddr=EE_DEV_ADDR;
        regaddr=iRegAddr;
    }
    else 
    {
        devaddr=EE_DEV_ADDR+((iRegAddr/256)<<1);
        regaddr=iRegAddr%256;
    }
	for(i=0;i<200;i++) //最多等待10ms
	{
		err=I2CM_Read(&EEP_I2CMInfo,devaddr,regaddr,&data,1);
        if(err==I2CM_EOK) break;
//        else rt_kprintf("[%s,%d] Dev=%x,Reg=%d,data=%d\n",__FUNCTION__,__LINE__,devaddr,regaddr,data);
		EEPROM_Delayus(50);
	}
    if(err!=I2CM_EOK)rt_kprintf("EEPROM_ReadByte err=%d\n",err);
	return data;
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  */
static void EEPROM_WriteByte(uint16_t iRegAddr, uint8_t iData)
{
	uint16_t i=0;
    uint8_t devaddr;
    uint16_t regaddr;
    eI2CM_Error err;

    if(EE_TYPE > AT24C16) 
    {
        devaddr=EE_DEV_ADDR;
        regaddr=iRegAddr;
    }
    else 
    {
        devaddr=EE_DEV_ADDR+((iRegAddr/256)<<1);
        regaddr=iRegAddr%256;
    }
	for(i=0;i<200;i++) //最多等待10ms
	{
		err=I2CM_Write(&EEP_I2CMInfo,devaddr,regaddr,&iData,1);
        if(err==I2CM_EOK) break;
//        else rt_kprintf("[%s,%d] Dev=%x,Reg=%d,data=%d\n",__FUNCTION__,__LINE__,devaddr,regaddr,iData);
		EEPROM_Delayus(50);
	}
    if(err!=I2CM_EOK)rt_kprintf("EEPROM_ReadByte err=%d\n",err);
}

void EEPROM_Init(void)
{
    uint8_t regbits=8;
    
    PRINT_INIT_START();
    if(EE_TYPE>AT24C16) regbits=16;
    I2CM_Init(&EEP_I2CMInfo,I2CM_IO_TYPE_MCU_IO, //I2C IO类型，FPGA的IO或者是MCU的IO
              8,regbits, //器件地址的位数(7或8) ， 寄存器位数(8或16)
              GPIOF,1,   //SCL Port , SCL Pin 如果是MCU的IO则填写端口及引脚，如果是FPGAIO则只用填写引脚，端口设置为RT_NULL
              GPIOF,0,   //SDA Port , SDA Pin 
              10,I2CM_WAIT_ACK,0); //延时时间，是否需要等待ACK
    EEPROM_ReadByte(0);
    PRINT_INIT_OK();
}


//在AT24CXX中指定地址开始读取指定个数的数据
void EEPROM_Read(uint16_t iRegAddr, uint8_t *iBuf, uint16_t iDataLen,uint16_t iBufLen)
{
    if(iDataLen>iBufLen) iDataLen=iBufLen;
	while(iDataLen)
	{
		*iBuf = EEPROM_ReadByte(iRegAddr);
		iRegAddr++;
		iBuf++;
		iDataLen--;
	}
}

//在AT24CXX中指定地址开始写入指定个数的数据
void EEPROM_Write(uint16_t iRegAddr, uint8_t *iData, uint16_t iLen)
{
    while(iLen)
	{
		EEPROM_WriteByte(iRegAddr, *iData);
		iRegAddr++;
		iData++;
		iLen--;
	}
}

void EEPROM_Wr8(uint16_t iAddr,uint8_t iData)
{
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%d\n",iAddr,iData);
    EEPROM_WriteByte(iAddr,iData);
}

void EEPROM_Wr16(uint16_t iAddr,uint16_t iData)
{
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%d\n",iAddr,iData);
    EEPROM_Write(iAddr,(uint8_t *)(&iData),2);
}

void EEPROM_Wr32(uint16_t iAddr,uint32_t iData)
{
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%d\n",iAddr,iData);
    EEPROM_Write(iAddr,(uint8_t *)(&iData),4);
}

void EEPROM_Wrf(uint16_t iAddr,float iData)
{
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%f\n",iAddr,iData);
    EEPROM_Write(iAddr,(uint8_t *)(&iData),4);
}

void EEPROM_WrStr(uint16_t iAddr,const char *iData)
{
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%s\n",iAddr,iData);
    EEPROM_Write(iAddr,(uint8_t *)(iData),strlen(iData));
}

uint8_t EEPROM_Rd8(uint16_t iAddr)		  
{
    uint8_t data;
	data=EEPROM_ReadByte(iAddr) ;
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%d\n",iAddr,data);
    return data;
}

uint16_t EEPROM_Rd16(uint16_t iAddr)		  
{
	uint16_t data;
    EEPROM_Read(iAddr,(uint8_t *)(&data),2,sizeof(data));
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%d\n",iAddr,data);
    return data;
}

uint32_t EEPROM_Rd32(uint16_t iAddr)		  
{
	uint32_t data;
    EEPROM_Read(iAddr,(uint8_t *)(&data),4,sizeof(data));
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%d\n",iAddr,data);
    return data;
}

float EEPROM_Rdf(uint16_t iAddr)		  
{
	float data;
    EEPROM_Read(iAddr,(uint8_t *)(&data),4,sizeof(data));
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%f\n",iAddr,data);
    return data;
}

void EEPROM_RdStr(uint16_t iAddr,char *iBuf,uint16_t iStrLen,uint16_t iBufLen)		  
{
    if(iStrLen>iBufLen) iStrLen=iBufLen;
    EEPROM_Read(iAddr,(uint8_t *)(iBuf),iStrLen,iBufLen);
    EEPROM_DEBUG(0x01,"Addr=%d,Data=%s\n",iAddr,iBuf);
}


/*-------------------------以下函数在xshell中使用----------------------------*/

static void xEEPROM_Wr8(uint16_t iAddr,uint8_t iData)
{
    EEPROM_Wr8(iAddr,iData);
}

static void xEEPROM_Wr16(uint16_t iAddr,uint16_t iData)
{
    EEPROM_Wr16(iAddr,iData);
}

static void xEEPROM_Wr32(uint16_t iAddr,uint32_t iData)
{
    EEPROM_Wr32(iAddr,iData);
}

static void xEEPROM_WrStr(uint16_t iAddr,const char *iData)
{
    EEPROM_WrStr(iAddr,iData);
}

static void xEEPROM_Wrf(uint16_t iAddr,const char *iData)
{
    EEPROM_Wrf(iAddr,atof(iData));
}

static void xEEPROM_Rd8(uint16_t iAddr)		  
{
	rt_kprintf("Addr=%d,Data=%d\n",iAddr,EEPROM_Rd8(iAddr));
}

static void xEEPROM_Rd16(uint16_t iAddr)		  
{
    rt_kprintf("Addr=%d,Data=%d\n",iAddr,EEPROM_Rd16(iAddr));
}

static void xEEPROM_Rd32(uint16_t iAddr)		  
{
    rt_kprintf("Addr=%d,Data=%d\n",iAddr,EEPROM_Rd32(iAddr));
}

static void xEEPROM_Rdf(uint16_t iAddr)		  
{
    rt_kprintf("Addr=%d,Data=%f\n",iAddr,EEPROM_Rdf(iAddr));
}

static void xEEPROM_RdStr(uint16_t iAddr,uint8_t iStrLen)		  
{
    uint8_t *buf;
    
    buf=rt_malloc(iStrLen+1);
    RT_ASSERT(buf != RT_NULL);
    memset(buf,0,iStrLen+1);
    
    EEPROM_Read(iAddr,(uint8_t *)(buf),iStrLen,iStrLen+1);
    rt_kprintf("%s\n",buf);
    rt_free(buf);
}


FINSH_FUNCTION_EXPORT(xEEPROM_Wr8,xEEPROM_Wr8(iAddr,iData))
FINSH_FUNCTION_EXPORT(xEEPROM_Wr16,xEEPROM_Wr16(iAddr,iData))
FINSH_FUNCTION_EXPORT(xEEPROM_Wr32,xEEPROM_Wr32(iAddr,iData))
FINSH_FUNCTION_EXPORT(xEEPROM_Wrf,xEEPROM_Wrf(iAddr,iData))
FINSH_FUNCTION_EXPORT(xEEPROM_WrStr,xEEPROM_WrStr(iAddr,iData))

FINSH_FUNCTION_EXPORT(xEEPROM_Rd8,xEEPROM_Rd8(iAddr))
FINSH_FUNCTION_EXPORT(xEEPROM_Rd16,xEEPROM_Rd16(iAddr))
FINSH_FUNCTION_EXPORT(xEEPROM_Rd32,xEEPROM_Rd32(iAddr))
FINSH_FUNCTION_EXPORT(xEEPROM_Rdf,xEEPROM_Rdf(iAddr))
FINSH_FUNCTION_EXPORT(xEEPROM_RdStr,xEEPROM_RdStr(iAddr))

