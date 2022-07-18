/*************************************************************************
 * File     : bsp_sub.h
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-10      aaron           first version
 ************************************************************************/
#ifndef __BSP_SUB_H
#define __BSP_SUB_H
#include "stdint.h"
#include "stdlib_bitoperator.h"

#define SCSI_PIN_NUM        64

#define ADDR_CLASS_OFFSET       14          //IO��ַ����ƫ����
#define ADDR_CLASS_MASK         0xC000      //IO��ַ�������� CPU����IO or FPGA�Ӱ�IO
#define ADDR_REG_OFFSET         8           //IO��ַ�Ĵ�����ַƫ����
#define ADDR_REG_MASK           0x3F00      //IO��ַ�Ĵ�����ַ���� 
#define ADDR_LOCATION_OFFSET    4           //IO��λƫ����
#define ADDR_LOCATION_MASK      0x00F0      //IO��λ��Ϣ����
#define ADDR_BIT_OFFSET         0           //IO��ַ����λƫ����
#define ADDR_BIT_MASK           0x000F      //IO��ַ����λ����

#define GPIO_OFFSET             0x0400      //CPU���ϵ�GPIO��ַƫ����

/*** FPGA SUB BOARD BUFFER ADDR �����üĴ�����64������ַ��0x00-0x3F***/
#define FSUB_REG_MAX_NUM 64  //FPGA�Ĵ�������
#define FSUB_GPO_DATA_REGADDR(n)    (0x00+n) //�Ĵ���0x00-0x07����8��Ϊ����Ĵ���
#define FSUB_GPO_EN_REGADDR(n)      (0x08+n) //�Ĵ���0x08-0x0F����8��Ϊ���ʹ�ܼĴ���
#define FSUB_GPI_DATA_REGADDR(n)    (0x10+n) //�Ĵ���0x10-0x17����8��Ϊ����Ĵ���
#define FSUB_GPI_ESW_ADDR(n)        (0x18+n) //�Ĵ���0x18-0x1F����8��Ϊ�ⲿ���ص�ַ
#define FSUB_FUNC_REGADDR(n)        (0x20+n) //�Ĵ���0x20-0x3F����32��Ϊ���ܼĴ���
#define FSUB_GPIO_REG_OFFSET_MASK   (0x07)   //IO�Ĵ���ƫ�ƣ��� FSUB_GPO_EN_REGADDR(n)&FSUB_GPIO_REG_OFFSET_MASK �õ��ľ��ǼĴ���ƫ����n
#define FUNCTYPE_ADDR           0x3E
#define FSUB_REV_ADDR           0x3F

#define GSW_TYPE                0               //���ذ����ͺ�
#define GIO_TYPE                1               //IO�����ͺ�
#define FUNC_TYPE(n)            (0x10+n)        //��FPGA���ܰ����ͺ�

/*�Ӱ�����*/
typedef enum
{
    SUB_BOARD_SWITCH_A,
    SUB_BOARD_IO,
    SUB_BOARD_MIC,
    SUB_BOARD_SWITCH_B,
    SUB_BOARD_GICA,
    SUB_BOARD_GLCA,
    
    SUB_BOARD_TYPE_NUM,
}eSubBoardType;

typedef enum
{
    SUB1_1 = 0,
    SUB1_2,
    SUB1_3,
    SUB1_4,
    SUB1_5,
    SUB1_6,
    SUB1_7,
    SUB2_1,
    SUB2_2,
    SUB2_3,
    SUB2_4,
    SUB2_5,
    SUB2_6,
    SUB2_7,
    SUB_BOARD_NUM,
}eSubSite;

typedef struct
{
    uint8_t Type;
    const char *Name;
    uint8_t Version;
    uint8_t Seq;//��ͬ��忨�еı�ţ���0��ʼ
}sSubBoardInfo;


typedef struct
{
    uint8_t currentType;
    uint8_t appendFunc;
}sSubInfo;

extern sSubBoardInfo SubBoardInfo[SUB_BOARD_NUM];

extern void SUB_Init(void);
extern void SUB_WriteByte(uint8_t regAddr, uint8_t data, eSubSite subSite);
extern uint8_t SUB_ReadByte(uint8_t regAddr, eSubSite subSite);
extern void SUB_SetBit(uint8_t iRegAddr, uint8_t iBit, eSubSite iSubSite);
extern void SUB_ResetBit(uint8_t iRegAddr, uint8_t iBit, eSubSite iSubSite);
extern void SUB_SetBits(uint8_t iRegAddr, uint8_t iBits, eSubSite iSubSite);
extern void SUB_ResetBits(uint8_t iRegAddr, uint8_t iBits, eSubSite iSubSite);
extern uint8_t SUB_GetBit(uint8_t iRegAddr, uint8_t iBit, eSubSite iSubSite);
/*ͨ���忨���ͼ���Ż�ȡ�忨���ڲ�λ��*/
extern eSubSite SUB_GetSiteFromSeq(eSubBoardType iBoardType,uint8_t iSeq);

#endif  // __BSP_SUB_H
