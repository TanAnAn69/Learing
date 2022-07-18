#ifndef __NET_ADDR_H
#define __NET_ADDR_H

#include "stdint.h"
#include "rtthread.h"

#define DEFAULT_LOCAL_IP3 100

enum
{
    CLIENT_UPPER_DATA_TRANS, //测试上位机：UDP协议，用于不规则的数据传输
    CLIENT_NUM,
};

//本地IP地址信息
#pragma pack(1)
typedef struct
{
    uint8_t BootFlag; // 0：下次重启进入用户代码 1:下次重启需进入boot代码
    uint8_t IP[4];    //IP地址
    uint8_t Mask[4];  //子网掩码
    uint8_t GW[4];    //网关
    uint8_t Mac[6];   //Mac地址
    char    BootVersion[5]; //boot程序版本号 V1.0  V1.1 
    char    BootKey[5];      //网络boot按键
    char    UserCodeType[8]; //用户代码类型 tester,table,box1等
    char    UserCodeVersion[16]; //用户代码版本
}sNetLocalAddr;

#pragma pack ()

typedef struct
{
    char *Name; //上位机名称：只要IP或者端口号中的任意一个不同，就认为是一个独立上位机
    uint8_t IP[4];
    uint16_t Port; //如果端口号是0，则说明客户端可以是任意端口号
    char *Desc; //描述
}sNetClientAddrInfo;


extern sNetLocalAddr  NetLocalAddr;
extern void NET_CfgInit(uint16_t iEepromAddr,uint16_t iEepromSize);
extern void NET_LocalDefault(uint8_t iIP);
extern void NET_ClientDefault(void);
extern void NET_LocalSave(void);
#endif  // __NET_ADDR_H
