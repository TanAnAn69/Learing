#ifndef __NETWORK_COMMU_H
#define __NETWORK_COMMU_H
#include "stdint.h"
#include "rtthread.h"
#include "sockets.h"

#define DBG_NETWORK_UDP_RECV 0x01
#define DBG_NETWORK_UDP_ACK  0x02
#define DBG_NETWORK_UDP_SEND 0x04
#define DBG_NETWORK_MBS_RECV 0x08
#define DBG_NETWORK_THREAD_REFRESH 0x80 //线程更新

/*-------------------------------------------------------------------------------------------------*
 *-------------------------------------------  UDP相关  -------------------------------------------*
 *-------------------------------------------------------------------------------------------------*/
#define NOT_ACK 0        //当前数据包不是ACK包
#define IS_ACK 1         //当前数据包是ACK包
#define NOT_NEED_ACK 0   //当前数据包不需要ACK
#define NEED_ACK 1       //当前数据包需要ACK
#define NOT_END_PACKET 0 //当前数据包不是最后一包
#define IS_END_PACKET 1 //当前数据包是最后一包

/*UDP配置信息*/
typedef struct
{
    uint16_t RecvBufLen;  //接收缓冲区长度，最小64，最大1400
    uint8_t *pRecvBuf;     //接收缓冲区指针
}sUDPCfgInfo;

/*udp接收数据信息*/
typedef struct
{
    struct rt_semaphore RecvSem;
    struct sockaddr_in SockAddr; //发送此数据包的远端主机Sock地址
    uint16_t RecvLen;//接收到的数据长度，以字节为单位
}sUDPRecvInfo;

extern sUDPRecvInfo UDPRecvInfo;

extern void UDPSend(uint8_t *iBuf,uint16_t iLen,struct sockaddr_in *iSockAddr);
extern void UDPPacketProcess(void);
extern void UDPSendAck(uint16_t iCmd,uint8_t *iBuf,uint16_t iDataLen,uint8_t iEndFlag,struct sockaddr_in *iSockAddr);

extern void NetworkCommuInit(sUDPCfgInfo *iUDPCfg);

#endif // __NETWORK_COMMU_H
