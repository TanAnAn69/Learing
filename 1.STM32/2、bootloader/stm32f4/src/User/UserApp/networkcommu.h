#ifndef __NETWORK_COMMU_H
#define __NETWORK_COMMU_H
#include "stdint.h"
#include "rtthread.h"
#include "sockets.h"

#define DBG_NETWORK_UDP_RECV 0x01
#define DBG_NETWORK_UDP_ACK  0x02
#define DBG_NETWORK_UDP_SEND 0x04
#define DBG_NETWORK_MBS_RECV 0x08
#define DBG_NETWORK_THREAD_REFRESH 0x80 //�̸߳���

/*-------------------------------------------------------------------------------------------------*
 *-------------------------------------------  UDP���  -------------------------------------------*
 *-------------------------------------------------------------------------------------------------*/
#define NOT_ACK 0        //��ǰ���ݰ�����ACK��
#define IS_ACK 1         //��ǰ���ݰ���ACK��
#define NOT_NEED_ACK 0   //��ǰ���ݰ�����ҪACK
#define NEED_ACK 1       //��ǰ���ݰ���ҪACK
#define NOT_END_PACKET 0 //��ǰ���ݰ��������һ��
#define IS_END_PACKET 1 //��ǰ���ݰ������һ��

/*UDP������Ϣ*/
typedef struct
{
    uint16_t RecvBufLen;  //���ջ��������ȣ���С64�����1400
    uint8_t *pRecvBuf;     //���ջ�����ָ��
}sUDPCfgInfo;

/*udp����������Ϣ*/
typedef struct
{
    struct rt_semaphore RecvSem;
    struct sockaddr_in SockAddr; //���ʹ����ݰ���Զ������Sock��ַ
    uint16_t RecvLen;//���յ������ݳ��ȣ����ֽ�Ϊ��λ
}sUDPRecvInfo;

extern sUDPRecvInfo UDPRecvInfo;

extern void UDPSend(uint8_t *iBuf,uint16_t iLen,struct sockaddr_in *iSockAddr);
extern void UDPPacketProcess(void);
extern void UDPSendAck(uint16_t iCmd,uint8_t *iBuf,uint16_t iDataLen,uint8_t iEndFlag,struct sockaddr_in *iSockAddr);

extern void NetworkCommuInit(sUDPCfgInfo *iUDPCfg);

#endif // __NETWORK_COMMU_H
