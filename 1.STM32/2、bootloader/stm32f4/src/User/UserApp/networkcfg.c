/*网络配置*/
#include "networkcfg.h"
#include "bsp_eeprom.h"
#include "finsh.h"
#include "networkcommu.h"
#include "stdio.h"
#include "limitcfg.h"
#include "bsp_common.h"
extern const char BootVersion[];
extern void NET_LocalSave(void);

sNetLocalAddr  NetLocalAddr; //本机IP地址信息

uint16_t NetEepromAddr,NetEepromSize;

void BootKeyCheck(char *iPin)
{
    if((iPin[0]!='P') ||
       (iPin[1]<'A')  || (iPin[1]>'I') || 
       (iPin[2]<'0')  || (iPin[2]>'9') || 
       (atoi(&iPin[2])>15))
    {
        rt_kprintf("boot key error \n");       
        snprintf(NetLocalAddr.BootKey,sizeof(NetLocalAddr.BootKey),"%s","PB8");
        NET_LocalSave();
        return;
    }
}

static void NET_LocalPrint(void)
{
    rt_kprintf("-------------------- Local IP addr information --------------------\n");
    rt_kprintf("BootFlag=%d\n",NetLocalAddr.BootFlag);
    rt_kprintf("Local IP:%d,%d,%d,%d\n",NetLocalAddr.IP[0],NetLocalAddr.IP[1],NetLocalAddr.IP[2],NetLocalAddr.IP[3]);
    rt_kprintf("Local Mask:%d,%d,%d,%d\n",NetLocalAddr.Mask[0],NetLocalAddr.Mask[1],NetLocalAddr.Mask[2],NetLocalAddr.Mask[3]);
    rt_kprintf("Local GW:%d,%d,%d,%d\n",NetLocalAddr.GW[0],NetLocalAddr.GW[1],NetLocalAddr.GW[2],NetLocalAddr.GW[3]);
    rt_kprintf("Local Mac:%d,%d,%d,%d,%d,%d\n\n",NetLocalAddr.Mac[0],NetLocalAddr.Mac[1],NetLocalAddr.Mac[2],NetLocalAddr.Mac[3],NetLocalAddr.Mac[4],NetLocalAddr.Mac[5]);
    rt_kprintf("boot version = %s\n",BootVersion);
    rt_kprintf("boot key = %s\n",NetLocalAddr.BootKey);
    rt_kprintf("User Code Type = %s\n",NetLocalAddr.UserCodeType);
    rt_kprintf("User Code Version = %s\n",NetLocalAddr.UserCodeVersion); 
    rt_kprintf("\ncall function NET_LocalDefault(100) in shell to restore the default settings\n");
}

void NET_CfgInit(uint16_t iEepromAddr,uint16_t iEepromSize)
{
    NetEepromAddr=iEepromAddr;
    NetEepromSize=iEepromSize;
    
    /*读取本地IP地址信息*/
    EEPROM_Read(iEepromAddr,(uint8_t *)(&NetLocalAddr),sizeof(NetLocalAddr),sizeof(NetLocalAddr));
    NetLocalAddr.Mac[5]=NetLocalAddr.IP[3];
    BootKeyCheck(NetLocalAddr.BootKey);
    /*打印地址信息*/
    NET_LocalPrint();
    
    if(strcmp(BootVersion,NetLocalAddr.BootVersion)!=0)
    {
        snprintf(NetLocalAddr.BootVersion,sizeof(NetLocalAddr.BootVersion),"%s",BootVersion); 
        NET_LocalSave();
    }
    
}

void NET_LocalSave(void)
{
    EEPROM_Write(NetEepromAddr,(uint8_t *)(&NetLocalAddr),  sizeof(NetLocalAddr));
    rt_kprintf("Save Done:%d Byte\n", sizeof(NetLocalAddr));
    NET_LocalPrint();
    
}

/*iIP：IP地址的最后一个字节*/
void NET_LocalDefault(uint8_t iIP)
{
    memset(&NetLocalAddr,0,sizeof(NetLocalAddr));
    NetLocalAddr.BootFlag=0;
    
    NetLocalAddr.IP[0]=192;
    NetLocalAddr.IP[1]=168;
    NetLocalAddr.IP[2]=0;
    NetLocalAddr.IP[3]=iIP;

    NetLocalAddr.Mask[0]=255;
    NetLocalAddr.Mask[1]=255;
    NetLocalAddr.Mask[2]=255;
    NetLocalAddr.Mask[3]=0;
    
    NetLocalAddr.GW[0]=192;
    NetLocalAddr.GW[1]=168;
    NetLocalAddr.GW[2]=0;
    NetLocalAddr.GW[3]=1;

    NetLocalAddr.Mac[0]=0x00;
    NetLocalAddr.Mac[1]=0x04;
    NetLocalAddr.Mac[2]=0xA3;
    NetLocalAddr.Mac[3]=0x11;
    NetLocalAddr.Mac[4]=0xA9;
    NetLocalAddr.Mac[5]=iIP;
    
    snprintf(NetLocalAddr.BootVersion,sizeof(NetLocalAddr.BootVersion),"%s",BootVersion);
    snprintf(NetLocalAddr.BootKey,sizeof(NetLocalAddr.BootKey),"%s","PB8");
    snprintf(NetLocalAddr.UserCodeType,sizeof(NetLocalAddr.UserCodeType),"%s","tester");
    snprintf(NetLocalAddr.UserCodeVersion,sizeof(NetLocalAddr.UserCodeVersion),"%s","V1.0.0_20190101");  
    NET_LocalSave();
}


/******************************************************************************************
* @Function:    Net_SetHostIP
* @Parameter:   [char*]
* @Return:      [None]
* @Description: 设置本机IP
*******************************************************************************************/
static void SetIP(uint8_t ip0,uint8_t ip1,uint8_t ip2,uint8_t ip3)
{
    NetLocalAddr.IP[0] = ip0;
    NetLocalAddr.IP[1] = ip1;
    NetLocalAddr.IP[2] = ip2;    
    NetLocalAddr.IP[3] = ip3;  
    NET_LocalSave();
}

/******************************************************************************************
* @Function:    Net_SetMask
* @Parameter:   [char*]
* @Return:      [None]
* @Description: 设置掩码
*******************************************************************************************/
static void SetMask(uint8_t mask0,uint8_t mask1,uint8_t mask2,uint8_t mask3)   //设置本机子网掩码
{
    NetLocalAddr.Mask[0] = mask0;
    NetLocalAddr.Mask[1] = mask1;
    NetLocalAddr.Mask[2] = mask2;
    NetLocalAddr.Mask[3] = mask3;
    NET_LocalSave();
}

/******************************************************************************************
* @Function:    Net_SetGW
* @Parameter:   [char*]
* @Return:      [None]
* @Description: 设置网关
*******************************************************************************************/
static void SetGW(uint8_t gw0,uint8_t gw1,uint8_t gw2,uint8_t gw3)   //设置本机子网掩码
{
    NetLocalAddr.GW[0] = gw0;
    NetLocalAddr.GW[1] = gw1;
    NetLocalAddr.GW[2] = gw2;    
    NetLocalAddr.GW[3] = gw3;
    NET_LocalSave();
}

static void SetMAC(uint8_t iMac0,uint8_t iMac1,uint8_t iMac2,uint8_t iMac3,uint8_t iMac4)
{
    NetLocalAddr.Mac[0]=iMac0;
    NetLocalAddr.Mac[1]=iMac1;
    NetLocalAddr.Mac[2]=iMac2;
    NetLocalAddr.Mac[3]=iMac3;
    NetLocalAddr.Mac[4]=iMac4;
    NetLocalAddr.Mac[5]=NetLocalAddr.IP[3];    
    NET_LocalSave();
}

FINSH_FUNCTION_EXPORT(NET_LocalDefault, NET_LocalDefault(100))
FINSH_FUNCTION_EXPORT(SetMask, SetMask(255,255,255,0))
FINSH_FUNCTION_EXPORT(SetGW, SetGW(192,168,100,1))
FINSH_FUNCTION_EXPORT(SetIP, SetIP(192,168,100,xxx))
FINSH_FUNCTION_EXPORT(SetMAC, SetMAC(0x00,0x04,0xA3,0x11,0xA9))

