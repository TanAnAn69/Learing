/************************************************************************
* 文件名称：stdlib_limit.c
* 当前版本：A01
* 文件描述： 
* 其它说明： 
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------------------------------
* 2018/12/26	  V1.0	    平台组	      实现基本功能
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>
#include "stm32f4xx.h"
#include "stdint.h"
#include "stdlib_limit.h"
#include "bsp_eeprom.h"
#include "finsh.h"
#include "bsp_common.h"

#define EEPROM_SIZE 8192   //64*1024/8  64Kbit
#define EEPROM_STARTADDRESS 0 
#define EEPROM_WriteStr(iAddr,iData,iLen)         EEPROM_Write(iAddr,iData,iLen)            //写指定长度字符串到EEPROM
#define EEPROM_ReadStr(iAddr,iData,iLen,iBufLen)  EEPROM_Read(iAddr,iData,iLen,iBufLen)     //从EEPROM读取指定长度字符串
#define EEPROM_WriteFloat(iAddr,iData)    EEPROM_Wrf(iAddr,iData)            //将一个float类型数据写入到EEPROM
#define EEPROM_ReadFloat(iAddr)           EEPROM_Rdf(iAddr)                  //读取一个float类型数据
#define EEPROM_Write8(iAddr,iData)     EEPROM_Wr8(iAddr,iData)            //将一个uint8类型数据写入到EEPROM
#define EEPROM_Read8(iAddr)            EEPROM_Rd8(iAddr)                  //读取一个uint8类型数据
#define EEPROM_Write16(iAddr,iData)    EEPROM_Wr16(iAddr,iData)            //将一个uint16类型数据写入到EEPROM
#define EEPROM_Read16(iAddr)           EEPROM_Rd16(iAddr)                  //读取一个uint16类型数据
#define EEPROM_Write32(iAddr,iData)    EEPROM_Wr32(iAddr,iData)            //将一个uint32类型数据写入到EEPROM
#define EEPROM_Read32(iAddr)           EEPROM_Rd32(iAddr)                  //读取一个uint32类型数据


static const sEepromBlockInfo *EepromBlock;//eeprom地址块分配信息


static  uint16_t EEPBItems;      //eeprom地址分块项目数  
/************************************************************************
* 函数名称：SCFG_Init
* 功能描述：门限及配置初始化
* 其它说明：此函数主要用于检查数组SCfg,Limit,EepromBlock 中的信息是否有误，如果信息有误程序会打印相关信息然后停止运行
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------------------------------
* 2018/12/26	  V1.0	    平台组        实现基本功能
************************************************************************/
void SCFG_Init(const sEepromBlockInfo *iEepromBlockInfo,uint16_t iEEPBItems)
{
    uint16_t usedsize;  //已用空间,可用空间    
    uint16_t i,j;

    PRINT_INIT_START();
    
    EepromBlock=iEepromBlockInfo;                                     //eeprom地址块分配信息
    EEPBItems=iEEPBItems;                                              //eeprom地址分块项目数  

    /******检查地址分块******/
    usedsize=0;
    for(i=0;i<EEPBItems;i++)
    {
        for(j=i+1;j<EEPBItems;j++)      //分块名称重复检查
        {
            if(strcmp(EepromBlock[i].Name,EepromBlock[j].Name)==0)    //是否重复
            {
                rt_kprintf("LINE:%d&%d, %s EEPROM地址分配名称重复\n",i,j,EepromBlock[i].Name);
            }
        }
        usedsize+=EepromBlock[i].Size;
    }
    if(usedsize>EEPROM_SIZE)             //分块大小检查
    {
        rt_kprintf("EEPROM地址分配超过EEPROM大小\n");
    }

    PRINT_INIT_OK();
}


/************************************************************************
* 函数名称：EEPB_GetAddr
* 功能描述：用于获取指定块的eeprom地址
* 输入参数：const char* iName:地址块名称  iAddr:指定块的eeprom地址  iSize:指定块的eeprom大小
* 返 回 值：指定块的eeprom地址
* 其它说明： 
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------------------------------
* 2018/12/26	  V1.0	    平台组        实现基本功能
************************************************************************/
uint16_t EEPB_GetAddr(const char* iName,uint16_t *iAddr,uint16_t *iSize)
{
    uint16_t i;
    uint16_t addr=EEPROM_STARTADDRESS;

    for(i=0; i<EEPBItems; i++)
    {
        if(strcmp(EepromBlock[i].Name, iName) == 0)
        {
            *iAddr=addr;
            *iSize=EepromBlock[i].Size;
            return addr;
        }
        addr+=EepromBlock[i].Size;
    }
    rt_kprintf("未找到 %s 的eeprom地址!\n", iName);
    return 0;
}
