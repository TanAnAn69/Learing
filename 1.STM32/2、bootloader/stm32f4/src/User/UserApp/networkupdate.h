#ifndef __NETWORK_UPDATE_H
#define __NETWORK_UPDATE_H
#include "stdint.h"
#define FILE_BUFFER_NUM 40

typedef enum
{
    FILE_STATE_INIT=0,  //��ʼ״̬
    FILE_STATE_RECEIVE_OK, //�������ݳɹ�
    FILE_STATE_DOWNLOAD_OK, //�������ݳɹ�
}eFileState;
typedef struct
{
    uint16_t Seq;
    uint16_t DataLen;
    uint8_t Data[1024];
    eFileState State;
}sFileBuf;


extern uint32_t CodeTotalLength; //���볤��
extern uint32_t CodeDownloadLength; //��д��flash�Ĵ��볤��
extern uint32_t CodeUpperCheckSum; //��λ������Ĵ���У���
extern uint32_t CodeLowerCheckSum; //��λ������Ĵ���У���
extern uint8_t CurrentProgress; //��ǰ����
extern uint8_t CheckSumResult;////0:��ʼ״̬ 1��������  2��У��ֵ����  3��У��ֵ��ȷ
extern uint8_t EraseFlashResult; // �����BYTE����0����ʼ״̬  1��������  2������ʧ��  3�������ɹ�
extern char CurrentState[32]; //��ǰ״̬
extern struct rt_semaphore UpdateSem;
extern sFileBuf FileBuf[FILE_BUFFER_NUM];


extern uint8_t SaveFileToBuf(uint16_t iSeq,uint8_t *iData,uint16_t iDataLen);
extern uint8_t EraseFlash(uint32_t iLen);

#endif // __NETWORK_UPDATE_H
