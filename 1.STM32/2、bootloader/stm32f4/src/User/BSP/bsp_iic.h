#ifndef __BSP_I2CM_H
#define __BSP_I2CM_H
#include "stdint.h"
#include "stm32f4xx.h"
#include "bsp_sub.h"

typedef enum
{
    I2CM_WAIT_ACK=0,
    I2CM_NOT_WAIT_ACK
}eI2CM_WaitACKOpt;

typedef enum
{
    I2CM_IO_TYPE_MCU_IO=0,
    I2CM_IO_TYPE_FPGA_IO
}eI2CM_IOType;

typedef struct {
    eI2CM_IOType   IOType;     //SCL和SDA需采用同一类型的IO
    uint8_t        DevAddrBits;//器件地址位数，7位或者是8位，如果输入的是7位地址，则程序会自动进行左移然后在最低位加读写标志
    uint8_t        RegAddrBits;  //I2C寄存器地址位数，一般为0/8/16，如果为0则不发送寄存器地址，如果为8则发送1字节的寄存器地址
	GPIO_TypeDef*  SCL_GPIO;   //SCL GPIO,如果是MCU的IO，则此处填写GPIOA~GPIOI，如果是FPGA的IO，则此处填写RT_NULL
	uint16_t	   SCL_Pin;    //SCL PIN，如果是MCU的IO，则此处填写0~15，如果是FPGA的IO，则此处填写对应的16进制的IO地址
	GPIO_TypeDef*  SDA_GPIO;   //SDA GPIO,如果是MCU的IO，则此处填写GPIOA~GPIOI，如果是FPGA的IO，则此处填写RT_NULL
	uint16_t	   SDA_Pin;    //SDA PIN，如果是MCU的IO，则此处填写0~15，如果是FPGA的IO，则此处填写对应的16进制的IO地址
	uint16_t 	   Delay;      //延时时间
    eI2CM_WaitACKOpt ACKFlag;  //是否需要等待ACK
    uint8_t        DebugOpt;  //调试选项
    
    uint8_t        InitFlag;    //初始化标志
    /*MCU IO配置*/
    uint16_t       SCL_Pin_x;  //对应GPIO_Pin_0~GPIO_Pin_15
    uint16_t       SDA_Pin_x;  //对应GPIO_Pin_0~GPIO_Pin_15
    /*FPGA IO配置*/
    eSubSite       FSCL_SubSite;//FPGA：子板编号
    uint8_t        FSCL_OutReg; //FPGA：输出寄存器
    uint8_t        FSCL_BitPos; //FPGA：引脚所在寄存器中的位置，取值0~7
    eSubSite       FSDA_SubSite;//FPGA：子板编号
    uint8_t        FSDA_CfgReg; //FPGA：当前IO的配置寄存器，用于配置输入输出模式
    uint8_t        FSDA_OutReg; //
    uint8_t        FSDA_InReg;  //
    uint8_t        FSDA_BitPos; //FPGA：引脚所在寄存器中的位置，取值0~7
}sI2CM_CfgInfo;


typedef enum{
	I2CM_EOK=0,
    I2CM_ACK_ERROR,
    I2CM_WDEV_ACKERR, //写入器件地址时，获取ACK失败（写）
    I2CM_REGH_ACKERR, //写入寄存器高地址时，获取ACK失败(仅当寄存器地址是16位时才会用到)（写）
    I2CM_REGL_ACKERR, //写入寄存器低地址时，获取ACK失败（写）
    I2CM_WDATA_ACKERR,//写入数据时，获取ACK失败（写）
    I2CM_RDEV_ACKERR, //写入器件地址时，获取ACK失败（读）
}eI2CM_Error;

extern void I2CM_Init(sI2CM_CfgInfo *iCfg,eI2CM_IOType iIOType,
                      uint8_t iDevAddrBits,uint8_t iRegAddrBits,
                      GPIO_TypeDef* iSCL_GPIO,uint16_t iSCL_Pin,
                      GPIO_TypeDef* iSDA_GPIO,uint16_t iSDA_Pin,
                      uint16_t iDelay,eI2CM_WaitACKOpt iACKFlag,uint8_t iDebugOpt);

extern eI2CM_Error I2CM_Read(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint16_t iRegAddr,uint8_t *iData,uint8_t iLen);
extern eI2CM_Error I2CM_Write(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint16_t iRegAddr,uint8_t *pWrite,uint8_t iLen);
extern eI2CM_Error I2CM_ReadOneByte(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint8_t iRegAddr,uint8_t *iData);
extern eI2CM_Error I2CM_WriteOneByte(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint8_t iRegAddr,uint8_t iData);
#endif
