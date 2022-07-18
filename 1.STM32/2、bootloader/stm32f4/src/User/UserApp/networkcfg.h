#ifndef __NET_ADDR_H
#define __NET_ADDR_H

#include "stdint.h"
#include "rtthread.h"

#define DEFAULT_LOCAL_IP3 100

enum
{
    CLIENT_UPPER_DATA_TRANS, //������λ����UDPЭ�飬���ڲ���������ݴ���
    CLIENT_NUM,
};

//����IP��ַ��Ϣ
#pragma pack(1)
typedef struct
{
    uint8_t BootFlag; // 0���´����������û����� 1:�´����������boot����
    uint8_t IP[4];    //IP��ַ
    uint8_t Mask[4];  //��������
    uint8_t GW[4];    //����
    uint8_t Mac[6];   //Mac��ַ
    char    BootVersion[5]; //boot����汾�� V1.0  V1.1 
    char    BootKey[5];      //����boot����
    char    UserCodeType[8]; //�û��������� tester,table,box1��
    char    UserCodeVersion[16]; //�û�����汾
}sNetLocalAddr;

#pragma pack ()

typedef struct
{
    char *Name; //��λ�����ƣ�ֻҪIP���߶˿ں��е�����һ����ͬ������Ϊ��һ��������λ��
    uint8_t IP[4];
    uint16_t Port; //����˿ں���0����˵���ͻ��˿���������˿ں�
    char *Desc; //����
}sNetClientAddrInfo;


extern sNetLocalAddr  NetLocalAddr;
extern void NET_CfgInit(uint16_t iEepromAddr,uint16_t iEepromSize);
extern void NET_LocalDefault(uint8_t iIP);
extern void NET_ClientDefault(void);
extern void NET_LocalSave(void);
#endif  // __NET_ADDR_H
