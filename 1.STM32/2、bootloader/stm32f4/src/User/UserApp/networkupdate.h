#ifndef __NETWORK_UPDATE_H
#define __NETWORK_UPDATE_H
#include "stdint.h"
#define FILE_BUFFER_NUM 40

typedef enum
{
    FILE_STATE_INIT=0,  //初始状态
    FILE_STATE_RECEIVE_OK, //接收数据成功
    FILE_STATE_DOWNLOAD_OK, //下载数据成功
}eFileState;
typedef struct
{
    uint16_t Seq;
    uint16_t DataLen;
    uint8_t Data[1024];
    eFileState State;
}sFileBuf;


extern uint32_t CodeTotalLength; //代码长度
extern uint32_t CodeDownloadLength; //已写入flash的代码长度
extern uint32_t CodeUpperCheckSum; //上位机计算的代码校验和
extern uint32_t CodeLowerCheckSum; //下位机计算的代码校验和
extern uint8_t CurrentProgress; //当前进度
extern uint8_t CheckSumResult;////0:初始状态 1：计算中  2：校验值错误  3：校验值正确
extern uint8_t EraseFlashResult; // 结果（BYTE）：0：初始状态  1：擦除中  2：擦除失败  3：擦除成功
extern char CurrentState[32]; //当前状态
extern struct rt_semaphore UpdateSem;
extern sFileBuf FileBuf[FILE_BUFFER_NUM];


extern uint8_t SaveFileToBuf(uint16_t iSeq,uint8_t *iData,uint16_t iDataLen);
extern uint8_t EraseFlash(uint32_t iLen);

#endif // __NETWORK_UPDATE_H
