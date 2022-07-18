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
    eI2CM_IOType   IOType;     //SCL��SDA�����ͬһ���͵�IO
    uint8_t        DevAddrBits;//������ַλ����7λ������8λ������������7λ��ַ���������Զ���������Ȼ�������λ�Ӷ�д��־
    uint8_t        RegAddrBits;  //I2C�Ĵ�����ַλ����һ��Ϊ0/8/16�����Ϊ0�򲻷��ͼĴ�����ַ�����Ϊ8����1�ֽڵļĴ�����ַ
	GPIO_TypeDef*  SCL_GPIO;   //SCL GPIO,�����MCU��IO����˴���дGPIOA~GPIOI�������FPGA��IO����˴���дRT_NULL
	uint16_t	   SCL_Pin;    //SCL PIN�������MCU��IO����˴���д0~15�������FPGA��IO����˴���д��Ӧ��16���Ƶ�IO��ַ
	GPIO_TypeDef*  SDA_GPIO;   //SDA GPIO,�����MCU��IO����˴���дGPIOA~GPIOI�������FPGA��IO����˴���дRT_NULL
	uint16_t	   SDA_Pin;    //SDA PIN�������MCU��IO����˴���д0~15�������FPGA��IO����˴���д��Ӧ��16���Ƶ�IO��ַ
	uint16_t 	   Delay;      //��ʱʱ��
    eI2CM_WaitACKOpt ACKFlag;  //�Ƿ���Ҫ�ȴ�ACK
    uint8_t        DebugOpt;  //����ѡ��
    
    uint8_t        InitFlag;    //��ʼ����־
    /*MCU IO����*/
    uint16_t       SCL_Pin_x;  //��ӦGPIO_Pin_0~GPIO_Pin_15
    uint16_t       SDA_Pin_x;  //��ӦGPIO_Pin_0~GPIO_Pin_15
    /*FPGA IO����*/
    eSubSite       FSCL_SubSite;//FPGA���Ӱ���
    uint8_t        FSCL_OutReg; //FPGA������Ĵ���
    uint8_t        FSCL_BitPos; //FPGA���������ڼĴ����е�λ�ã�ȡֵ0~7
    eSubSite       FSDA_SubSite;//FPGA���Ӱ���
    uint8_t        FSDA_CfgReg; //FPGA����ǰIO�����üĴ��������������������ģʽ
    uint8_t        FSDA_OutReg; //
    uint8_t        FSDA_InReg;  //
    uint8_t        FSDA_BitPos; //FPGA���������ڼĴ����е�λ�ã�ȡֵ0~7
}sI2CM_CfgInfo;


typedef enum{
	I2CM_EOK=0,
    I2CM_ACK_ERROR,
    I2CM_WDEV_ACKERR, //д��������ַʱ����ȡACKʧ�ܣ�д��
    I2CM_REGH_ACKERR, //д��Ĵ����ߵ�ַʱ����ȡACKʧ��(�����Ĵ�����ַ��16λʱ�Ż��õ�)��д��
    I2CM_REGL_ACKERR, //д��Ĵ����͵�ַʱ����ȡACKʧ�ܣ�д��
    I2CM_WDATA_ACKERR,//д������ʱ����ȡACKʧ�ܣ�д��
    I2CM_RDEV_ACKERR, //д��������ַʱ����ȡACKʧ�ܣ�����
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
