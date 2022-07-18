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

#define ADDR_CLASS_OFFSET       14          //IO地址类型偏移量
#define ADDR_CLASS_MASK         0xC000      //IO地址类型掩码 CPU主板IO or FPGA子板IO
#define ADDR_REG_OFFSET         8           //IO地址寄存器地址偏移量
#define ADDR_REG_MASK           0x3F00      //IO地址寄存器地址掩码 
#define ADDR_LOCATION_OFFSET    4           //IO槽位偏移量
#define ADDR_LOCATION_MASK      0x00F0      //IO槽位信息掩码
#define ADDR_BIT_OFFSET         0           //IO地址比特位偏移量
#define ADDR_BIT_MASK           0x000F      //IO地址比特位掩码

#define GPIO_OFFSET             0x0400      //CPU板上的GPIO基址偏移量

/*** FPGA SUB BOARD BUFFER ADDR ：可用寄存器共64个，地址从0x00-0x3F***/
#define FSUB_REG_MAX_NUM 64  //FPGA寄存器个数
#define FSUB_GPO_DATA_REGADDR(n)    (0x00+n) //寄存器0x00-0x07：共8个为输出寄存器
#define FSUB_GPO_EN_REGADDR(n)      (0x08+n) //寄存器0x08-0x0F：共8个为输出使能寄存器
#define FSUB_GPI_DATA_REGADDR(n)    (0x10+n) //寄存器0x10-0x17：共8个为输入寄存器
#define FSUB_GPI_ESW_ADDR(n)        (0x18+n) //寄存器0x18-0x1F：共8个为外部开关地址
#define FSUB_FUNC_REGADDR(n)        (0x20+n) //寄存器0x20-0x3F：共32个为功能寄存器
#define FSUB_GPIO_REG_OFFSET_MASK   (0x07)   //IO寄存器偏移，如 FSUB_GPO_EN_REGADDR(n)&FSUB_GPIO_REG_OFFSET_MASK 得到的就是寄存器偏移量n
#define FUNCTYPE_ADDR           0x3E
#define FSUB_REV_ADDR           0x3F

#define GSW_TYPE                0               //开关板类型号
#define GIO_TYPE                1               //IO板类型号
#define FUNC_TYPE(n)            (0x10+n)        //带FPGA功能板类型号

/*子板类型*/
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
    uint8_t Seq;//在同类板卡中的编号，从0开始
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
/*通过板卡类型及序号获取板卡所在槽位号*/
extern eSubSite SUB_GetSiteFromSeq(eSubBoardType iBoardType,uint8_t iSeq);

#endif  // __BSP_SUB_H
