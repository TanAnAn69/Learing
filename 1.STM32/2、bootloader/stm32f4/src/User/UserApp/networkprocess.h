/*
MSCR :  modbus slave controller register , readonly
MSCRV :  modbus slave controller register , readonly, register value 
MSCW = modbus slave controller register , writeonly
MSCWV = modbus slave controller register , writeonly, register value 

*/
#ifndef __NETWORK_PROCESS_H
#define __NETWORK_PROCESS_H
#include "networkcommu.h"

/*-------------------------------------------------------------------------------------------------*
 *------------------------------------------  UDP相关  -----------------------------------------*
 *-------------------------------------------------------------------------------------------------*/
#define CMD_BROADCAST                 0x0101
#define CMD_ENTER_BOOT                0x0102
#define CMD_ENTER_USER                0x0103
#define CMD_ERASE_FLASH               0x0104
#define CMD_UQARY_ERASE_FLASH_RESULT  0x0105
#define CMD_DOWNLOAD                  0x0106
#define CMD_QUARY_CHECK_SUM           0x0107

#define UDP_RECV_BUF_LEN 1100
 /*UDP数据包头信息*/
#pragma pack(1)
typedef struct
{
    uint8_t Head;
    uint8_t PacketLenL;
    uint8_t PacketLenH;
    uint8_t MajorCmd;
    uint8_t MinorCmd;
    uint8_t FrameIndexL;
    uint8_t FrameIndexH;
    uint8_t PacketIndexL;
    uint8_t PacketIndexH;
    uint8_t IsEndPacket;
    uint8_t IsAck;
    uint8_t IsNeedAck;
    uint8_t Resv[4];
    uint8_t Content[UDP_RECV_BUF_LEN-16];
}sUDPPacketFrame;
#pragma pack ()

#endif // __NETWORK_PROCESS_H
