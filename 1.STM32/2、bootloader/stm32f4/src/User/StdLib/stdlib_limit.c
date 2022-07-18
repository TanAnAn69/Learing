/************************************************************************
* �ļ����ƣ�stdlib_limit.c
* ��ǰ�汾��A01
* �ļ������� 
* ����˵���� 
* �޸�����        �汾��     �޸���	      �޸�����
* -----------------------------------------------------------------------
* 2018/12/26	  V1.0	    ƽ̨��	      ʵ�ֻ�������
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
#define EEPROM_WriteStr(iAddr,iData,iLen)         EEPROM_Write(iAddr,iData,iLen)            //дָ�������ַ�����EEPROM
#define EEPROM_ReadStr(iAddr,iData,iLen,iBufLen)  EEPROM_Read(iAddr,iData,iLen,iBufLen)     //��EEPROM��ȡָ�������ַ���
#define EEPROM_WriteFloat(iAddr,iData)    EEPROM_Wrf(iAddr,iData)            //��һ��float��������д�뵽EEPROM
#define EEPROM_ReadFloat(iAddr)           EEPROM_Rdf(iAddr)                  //��ȡһ��float��������
#define EEPROM_Write8(iAddr,iData)     EEPROM_Wr8(iAddr,iData)            //��һ��uint8��������д�뵽EEPROM
#define EEPROM_Read8(iAddr)            EEPROM_Rd8(iAddr)                  //��ȡһ��uint8��������
#define EEPROM_Write16(iAddr,iData)    EEPROM_Wr16(iAddr,iData)            //��һ��uint16��������д�뵽EEPROM
#define EEPROM_Read16(iAddr)           EEPROM_Rd16(iAddr)                  //��ȡһ��uint16��������
#define EEPROM_Write32(iAddr,iData)    EEPROM_Wr32(iAddr,iData)            //��һ��uint32��������д�뵽EEPROM
#define EEPROM_Read32(iAddr)           EEPROM_Rd32(iAddr)                  //��ȡһ��uint32��������


static const sEepromBlockInfo *EepromBlock;//eeprom��ַ�������Ϣ


static  uint16_t EEPBItems;      //eeprom��ַ�ֿ���Ŀ��  
/************************************************************************
* �������ƣ�SCFG_Init
* �������������޼����ó�ʼ��
* ����˵�����˺�����Ҫ���ڼ������SCfg,Limit,EepromBlock �е���Ϣ�Ƿ����������Ϣ���������ӡ�����ϢȻ��ֹͣ����
* �޸�����        �汾��     �޸���	      �޸�����
* -----------------------------------------------------------------------
* 2018/12/26	  V1.0	    ƽ̨��        ʵ�ֻ�������
************************************************************************/
void SCFG_Init(const sEepromBlockInfo *iEepromBlockInfo,uint16_t iEEPBItems)
{
    uint16_t usedsize;  //���ÿռ�,���ÿռ�    
    uint16_t i,j;

    PRINT_INIT_START();
    
    EepromBlock=iEepromBlockInfo;                                     //eeprom��ַ�������Ϣ
    EEPBItems=iEEPBItems;                                              //eeprom��ַ�ֿ���Ŀ��  

    /******����ַ�ֿ�******/
    usedsize=0;
    for(i=0;i<EEPBItems;i++)
    {
        for(j=i+1;j<EEPBItems;j++)      //�ֿ������ظ����
        {
            if(strcmp(EepromBlock[i].Name,EepromBlock[j].Name)==0)    //�Ƿ��ظ�
            {
                rt_kprintf("LINE:%d&%d, %s EEPROM��ַ���������ظ�\n",i,j,EepromBlock[i].Name);
            }
        }
        usedsize+=EepromBlock[i].Size;
    }
    if(usedsize>EEPROM_SIZE)             //�ֿ��С���
    {
        rt_kprintf("EEPROM��ַ���䳬��EEPROM��С\n");
    }

    PRINT_INIT_OK();
}


/************************************************************************
* �������ƣ�EEPB_GetAddr
* �������������ڻ�ȡָ�����eeprom��ַ
* ���������const char* iName:��ַ������  iAddr:ָ�����eeprom��ַ  iSize:ָ�����eeprom��С
* �� �� ֵ��ָ�����eeprom��ַ
* ����˵���� 
* �޸�����        �汾��     �޸���	      �޸�����
* -----------------------------------------------------------------------
* 2018/12/26	  V1.0	    ƽ̨��        ʵ�ֻ�������
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
    rt_kprintf("δ�ҵ� %s ��eeprom��ַ!\n", iName);
    return 0;
}
