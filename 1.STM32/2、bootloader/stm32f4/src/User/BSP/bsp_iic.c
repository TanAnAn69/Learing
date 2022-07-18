/*************************************************************************
 * File     : bsp_iic.c
 * Author   : kite
 * COPYRIGHT (C) 2017, YANMADE kite
 *
 * Description: I2CM standard interface;I2CM - I2C Master
 *
 * Change Logs:
 * Date            Author          Notes
 * 
 ************************************************************************/
#include "string.h"
#include "stdlib.h"
#include "stdio.h" 
#include <math.h>
#include "rtthread.h"
#include "bsp_iic.h"
#include "bsp_spi.h"
#include "bsp_common.h"

FILE_VERSION_EXPORT(bsp_iic,V1.0_20190312) 


/*��ʼ���ж�*/
#define Init_Judge(Cfg,format,...)	if(Cfg->InitFlag != 1){rt_kprintf("[I2CM Init Statues:] "format"",##__VA_ARGS__);assert_param(0);}
/*������Ϣ*/
#define I2CM_DEBUG(opt,format,...) 	if(iCfg->DebugOpt&opt){rt_kprintf("[I2CM:%02x] "format"",opt,##__VA_ARGS__);}

/*****************************************************/
/*** MACRO DEFINITION AREA ***/
/*****************************************************/
/*===================================�û��Զ������=====================================*/
#define I2CM_DEBUG_ENABLE 				0	//���Թ��ߣ�Ϊ1ʱ�򿪵��Կ���
#define CATCH_ACK_SWITCH				0	//ACK���Կ��أ�Ϊ1ʱ�򿪿���ר�Ų���ACK
/*======================================================================================*/

/*-----------------------------------MCU I2C����------------------------------------*/

/*��ַλ���ж�*/
#define Address_Judge(Cfg,addr)			if(Cfg->DevAddrBits==7){addr <<= 1; addr &= 0XFE;}else{addr &= 0XFE;}

/*ACK����*/
#define CATCH_ACK_PRINT					if(CATCH_ACK_SWITCH==1)rt_kprintf("***************ACK COME************** \n");

#define SCL1(Cfg)			  ( (Cfg->SCL_GPIO)->BSRRL=Cfg->SCL_Pin_x )
#define SCL0(Cfg)			  ( (Cfg->SCL_GPIO)->BSRRH=Cfg->SCL_Pin_x )
#define SDA1(Cfg)			  ( (Cfg->SDA_GPIO)->BSRRL=Cfg->SDA_Pin_x )
#define SDA0(Cfg)			  ( (Cfg->SDA_GPIO)->BSRRH=Cfg->SDA_Pin_x )

#define SDA_IN(Cfg)      ( Cfg->SDA_GPIO->MODER &= ~( GPIO_MODER_MODER0<<((iCfg->SDA_Pin)<<1) ) )
#define SDA_OUT(Cfg)     ( Cfg->SDA_GPIO->MODER |=  (uint32_t)( GPIO_Mode_OUT<<((iCfg->SDA_Pin)<<1) ) )
#define SDA_READ(Cfg)	  (Cfg->SDA_GPIO->IDR & Cfg->SDA_Pin_x)


/*-----------------------------------FPGA I2C����------------------------------------*/
                                        
#define FSCL1(Cfg)     // SUB_SetBit(Cfg->FSCL_OutReg,Cfg->FSCL_BitPos,Cfg->FSCL_SubSite)
#define FSCL0(Cfg)     // SUB_ResetBit(Cfg->FSCL_OutReg,Cfg->FSCL_BitPos,Cfg->FSCL_SubSite)
#define FSDA1(Cfg)     // SUB_SetBit(Cfg->FSDA_OutReg,Cfg->FSDA_BitPos,Cfg->FSDA_SubSite)
#define FSDA0(Cfg)     // SUB_ResetBit(Cfg->FSDA_OutReg,Cfg->FSDA_BitPos,Cfg->FSDA_SubSite)

/*����������Ӧ�����üĴ�����Ӧλ����Ϊ1�����������������Ϊ0������������*/
#define FSDA_IN(Cfg)   // SUB_ResetBit(Cfg->FSDA_CfgReg,Cfg->FSDA_BitPos,Cfg->FSDA_SubSite)
#define FSDA_OUT(Cfg)  // SUB_SetBit(Cfg->FSDA_CfgReg,Cfg->FSDA_BitPos,Cfg->FSDA_SubSite)
#define FSDA_READ(Cfg) (0)// SUB_GetBit(Cfg->FSDA_InReg,Cfg->FSDA_BitPos,Cfg->FSDA_SubSite)

void I2CM_Init(sI2CM_CfgInfo *iCfg,eI2CM_IOType iIOType,
               uint8_t iDevAddrBits,uint8_t iRegAddrBits,
               GPIO_TypeDef* iSCL_GPIO,uint16_t iSCL_Pin,
               GPIO_TypeDef* iSDA_GPIO,uint16_t iSDA_Pin,
               uint16_t iDelay,eI2CM_WaitACKOpt iACKFlag,uint8_t iDebugOpt)
{
    ///todo:����д:����������
    
    memset(iCfg,0,sizeof(sI2CM_CfgInfo));
    iCfg->IOType=iIOType;
    iCfg->RegAddrBits=iRegAddrBits;
    iCfg->DevAddrBits=iDevAddrBits;
    iCfg->SCL_GPIO=iSCL_GPIO;
    iCfg->SCL_Pin=iSCL_Pin;
    iCfg->SDA_GPIO=iSDA_GPIO;
    iCfg->SDA_Pin=iSDA_Pin;
    iCfg->Delay=iDelay;
    iCfg->ACKFlag=iACKFlag;
    iCfg->DebugOpt=iDebugOpt;
    
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        GPIO_InitTypeDef	GPIO_InitStructure;
               
        iCfg->SCL_Pin_x=(uint16_t)(1 << (iCfg->SCL_Pin) );
        iCfg->SDA_Pin_x=(uint16_t)(1 << (iCfg->SDA_Pin) );
        
        RCC_AHB1PeriphClockCmd( (uint32_t)( 1 << ( ((uint32_t)iCfg->SCL_GPIO-(uint32_t)GPIOA)/0x400 ) ), ENABLE);
        RCC_AHB1PeriphClockCmd( (uint32_t)( 1 << ( ((uint32_t)iCfg->SDA_GPIO-(uint32_t)GPIOA)/0x400 ) ), ENABLE);
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

        GPIO_InitStructure.GPIO_Pin =iCfg->SCL_Pin_x;
        GPIO_Init(iCfg->SCL_GPIO, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = iCfg->SDA_Pin_x;
        GPIO_Init(iCfg->SDA_GPIO, &GPIO_InitStructure);
        
        SCL1(iCfg);       //SCL��1
        SDA1(iCfg);       //SDA��1       
        
    }
    else
    {
//        iCfg->FSCL_SubSite = (eSubSite) (((iCfg->SCL_Pin & ADDR_LOCATION_MASK) >> ADDR_LOCATION_OFFSET)-1);
//        iCfg->FSDA_SubSite = (eSubSite) (((iCfg->SDA_Pin & ADDR_LOCATION_MASK) >> ADDR_LOCATION_OFFSET)-1);
//        
//        iCfg->FSCL_OutReg = (iCfg->SCL_Pin & ADDR_REG_MASK) >> ADDR_REG_OFFSET;  
//        iCfg->FSDA_OutReg = ((iCfg->SDA_Pin & ADDR_REG_MASK) >> ADDR_REG_OFFSET)&FSUB_GPIO_REG_OFFSET_MASK;//�˴��õ����ǼĴ���ƫ����
//        iCfg->FSDA_CfgReg = FSUB_GPO_EN_REGADDR(iCfg->FSDA_OutReg);
//        iCfg->FSDA_InReg = FSUB_GPI_DATA_REGADDR(iCfg->FSDA_OutReg);
//        iCfg->FSDA_OutReg = FSUB_GPO_DATA_REGADDR(iCfg->FSDA_OutReg);
//        
//        iCfg->FSCL_BitPos = (iCfg->SCL_Pin & ADDR_BIT_MASK) >> ADDR_BIT_OFFSET;
//        iCfg->FSDA_BitPos = (iCfg->SDA_Pin & ADDR_BIT_MASK) >> ADDR_BIT_OFFSET;
//        
//        FSCL1(iCfg);       //SCL��1
//        FSDA1(iCfg);       //SDA��1
    }
    
    iCfg->InitFlag=1;
    rt_kprintf("I2CM Init OK,Cfg RAM Addr=0x%08X,DebugOpt RAM Addr=0x%08X\n",iCfg,&(iCfg->DebugOpt));
    rt_kprintf("Debug Example: I2CM_ShowCfgInfo(0x%08X);  SetRAM8(0x%08X,0xff);\n",iCfg,&(iCfg->DebugOpt));
}

static void I2CM_DelayTime(uint16_t iDelay)
{
    if(iDelay==0) return;
    else
    {
        uint16_t i=0,k=0;
        for(i=0;i<iDelay;i++) k++;
    }
}

/**
  * @brief  .I2C��ʼ 
  * @note   .I2C���俪ʼ�ź� 
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  * @retval .void
  */
static void I2CM_Start(sI2CM_CfgInfo *iCfg)
{
    /*��ʼ�źţ�SCLΪ�ߵ�ƽ�ڼ䣬SDA�ɸߵ�ƽ��Ϊ�͵�ƽ
        SDA ----
                \
                 \
                  -------------
        SCL ------------
                        \
                         \
                          -----
           | -start- |
	*/
	
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_OUT(iCfg); 
        SDA1(iCfg);
        SCL1(iCfg);

        I2CM_DelayTime(iCfg->Delay);
        SDA0(iCfg);
        I2CM_DelayTime(iCfg->Delay);
        SCL0(iCfg);
        I2CM_DelayTime(iCfg->Delay);
    }
    else
    {
//        FSDA_OUT(iCfg); 
//        FSDA1(iCfg);
//        FSCL1(iCfg);

//        I2CM_DelayTime(iCfg->Delay);
//        FSDA0(iCfg);
//        I2CM_DelayTime(iCfg->Delay);
//        FSCL0(iCfg);        
//        I2CM_DelayTime(iCfg->Delay);
    }
}

/**
  * @brief  .I2C����
  * @note   .I2C���������ź�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  * @retval .void
  */
static void I2CM_Stop(sI2CM_CfgInfo *iCfg)
{  
    /*�����źţ�SCLΪ�ߵ�ƽ�ڼ䣬SDA�ɵ͵�ƽ��Ϊ�ߵ�ƽ
        SDA                 ---------
                           /
                          /
             -------------
        SCL         ---------------
                   /
                  / 
             -----
                     | -end- |
	*/
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_OUT(iCfg);
        SCL0(iCfg); 
        SDA0(iCfg);    
        I2CM_DelayTime(iCfg->Delay);
        
        SCL1(iCfg);
        I2CM_DelayTime(iCfg->Delay);
        SDA1(iCfg);
        I2CM_DelayTime(iCfg->Delay);    
    }
    else
    {
//        FSDA_OUT(iCfg);
//        FSCL0(iCfg); 
//        FSDA0(iCfg);    
//        I2CM_DelayTime(iCfg->Delay);
//        
//        FSCL1(iCfg);
//        I2CM_DelayTime(iCfg->Delay);
//        FSDA1(iCfg);
//        I2CM_DelayTime(iCfg->Delay);    
    }
}

/**
  * @brief  .I2C����һ���ֽ�
  * @note   .��������ʼ�����źŵ�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  *         .txd:��Ҫ���͵�һ���ֽ�
  * @retval .void
  */
static void I2CM_SendByte(sI2CM_CfgInfo *iCfg,uint8_t txd)
{
    uint8_t i;
	
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_OUT(iCfg);
        for(i = 0; i < 8; i++)
        {
            if(txd & 0x80)
                SDA1(iCfg);
            else
                SDA0(iCfg);
            
            txd <<= 1;
            I2CM_DelayTime(iCfg->Delay);
            SCL1(iCfg);
            I2CM_DelayTime(iCfg->Delay); 
            SCL0(iCfg);
        }
        I2CM_DelayTime(iCfg->Delay);   
    }
    else
    {
//        FSDA_OUT(iCfg);
//        for(i = 0; i < 8; i++)
//        {
//            if(txd & 0x80)
//                FSDA1(iCfg);
//            else
//                FSDA0(iCfg);
//            
//            txd <<= 1;
//            I2CM_DelayTime(iCfg->Delay);
//            FSCL1(iCfg);
//            I2CM_DelayTime(iCfg->Delay); 
//            FSCL0(iCfg);
//        }
//        I2CM_DelayTime(iCfg->Delay);           
    }
}


/**
  * @brief  .I2C��ACK�ź�
  * @note   .���ڷ���״̬ʱ�����ڽ��մӻ�Ӧ���ź�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  * @retval .ACK����״̬
  */
static uint8_t I2CM_Wait_Ack(sI2CM_CfgInfo *iCfg)
{
    uint16_t errorTime = 0;
    /*�ȴ��ӻ����͵�ACK�źţ��ӻ��ڵ�9��������֮ǰ����SDA��Ϊ�͵�ƽ�����ڵ�9������ĸߵ�ƽ�ڼ䱣��Ϊ�͵�ƽ*/

    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_IN(iCfg);
        SCL1(iCfg);
        I2CM_DelayTime(iCfg->Delay);
        if(iCfg->ACKFlag == I2CM_WAIT_ACK)
        {
            while(SDA_READ(iCfg))
            {
                errorTime++;
                if(errorTime > 20)
                {
                    I2CM_Stop(iCfg);
                    return I2CM_ACK_ERROR;
                }
                I2CM_DelayTime(iCfg->Delay);
            }
        }
        SCL0(iCfg);
        I2CM_DelayTime(iCfg->Delay);
        CATCH_ACK_PRINT;
    }
    else
    {
//        FSDA_IN(iCfg);
//        FSCL1(iCfg);
//        I2CM_DelayTime(iCfg->Delay);
//        if(iCfg->ACKFlag == I2CM_WAIT_ACK)
//        {
//            while(FSDA_READ(iCfg))
//            {
//                errorTime++;
//                if(errorTime > 200)
//                {
//                    I2CM_Stop(iCfg);
//                    return I2CM_ACK_ERROR;
//                }
//                I2CM_DelayTime(iCfg->Delay);
//            }
//        }
//        FSCL0(iCfg);
//        I2CM_DelayTime(iCfg->Delay);
//        CATCH_ACK_PRINT;
        
    }
    return I2CM_EOK;
}

/**
  * @brief  .ACK�ź� 
  * @note   .���մӻ����ݺ��������͵�ACKӦ���źţ�����SDA����
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  * @retval .void
  */
static void I2CM_Ack(sI2CM_CfgInfo *iCfg)
{
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_OUT(iCfg);
        
        SCL0(iCfg);
        SDA0(iCfg);
        I2CM_DelayTime(iCfg->Delay);

        SCL1(iCfg);
        I2CM_DelayTime(iCfg->Delay);

        SCL0(iCfg);
        I2CM_DelayTime(iCfg->Delay);
    }
    else
    {
//        FSDA_OUT(iCfg);
//        
//        FSCL0(iCfg);
//        FSDA0(iCfg);
//        I2CM_DelayTime(iCfg->Delay);

//        FSCL1(iCfg);
//        I2CM_DelayTime(iCfg->Delay);

//        FSCL0(iCfg);
//        I2CM_DelayTime(iCfg->Delay);        
    }
}

/**
  * @brief  .��ACK�ź� 
  * @note   .�����������ݺ󣬲�����ACK�źţ�����SDA��λ
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  * @retval .void
  */
static void I2CM_NAck(sI2CM_CfgInfo *iCfg)
{
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_OUT(iCfg);
        
        SCL0(iCfg);
        I2CM_DelayTime(iCfg->Delay);
        SDA1(iCfg);
        I2CM_DelayTime(iCfg->Delay);

        SCL1(iCfg);
        I2CM_DelayTime(iCfg->Delay);

        SCL0(iCfg);
    }
    else
    {
//        FSDA_OUT(iCfg);
//        
//        FSCL0(iCfg);
//        I2CM_DelayTime(iCfg->Delay);
//        FSDA1(iCfg);
//        I2CM_DelayTime(iCfg->Delay);

//        FSCL1(iCfg);
//        I2CM_DelayTime(iCfg->Delay);

//        FSCL0(iCfg);
    }
}


/**
  * @brief  .I2C����һ���ֽ�
  * @note   .��������ʼ�����źŵ�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  *         .ack:�Ƿ���ACK�ź�(1:����  0:������)
  * @retval .��ȡ����һ���ֽ�����
  */
static uint8_t I2CM_ReadByte(sI2CM_CfgInfo *iCfg,unsigned char ack)
{
    unsigned char i, receive = 0;

    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        SDA_IN(iCfg);
        I2CM_DelayTime(iCfg->Delay);

        for(i = 0; i < 8; i++)
        {
            SCL0(iCfg);
            I2CM_DelayTime(iCfg->Delay);
            SCL1(iCfg);
            receive <<= 1;
            if(SDA_READ(iCfg)) receive++;
            I2CM_DelayTime(iCfg->Delay);
        }
        SCL0(iCfg);
        
        if(ack == 0)
            I2CM_NAck(iCfg);
        else
            I2CM_Ack(iCfg);
    }
    else
    {
//        FSDA_IN(iCfg);
//        I2CM_DelayTime(iCfg->Delay);

//        for(i = 0; i < 8; i++)
//        {
//            FSCL0(iCfg);
//            I2CM_DelayTime(iCfg->Delay);
//            FSCL1(iCfg);
//            receive <<= 1;
//            if(FSDA_READ(iCfg)) receive++;
//            I2CM_DelayTime(iCfg->Delay);
//        }
//        FSCL0(iCfg);
//        
//        if(ack == 0)
//            I2CM_NAck(iCfg);
//        else
//            I2CM_Ack(iCfg);
    }
    return receive;
}

/**
  * @brief  .I2Cд���ֽں���
  * @note   .������ʼ�����źŵ�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  *			.pWrite:�������ݻ�������ʼ��ַ
  *			.iLen:���͵��ֽ���
  * @retval .д״̬��־
  */
eI2CM_Error I2CM_Write(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint16_t iRegAddr,uint8_t *pWrite,uint8_t iLen)
{
	uint8_t i;
	uint8_t ackflag=0;

	Init_Judge(iCfg,"WRITE I2CM is no init (thread=%s)\n",rt_thread_self()->name);	
    
	Address_Judge(iCfg,iDevAddr);
				   	  	    																 
    I2CM_Start(iCfg);  
	I2CM_SendByte(iCfg,iDevAddr);		//���豸��ַ
	ackflag=I2CM_Wait_Ack(iCfg);
	I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,Data[0]=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,pWrite[0],iLen);
    if(ackflag==I2CM_ACK_ERROR) return I2CM_WDEV_ACKERR;
    
    if(iCfg->RegAddrBits>0)
    {
        if(iCfg->RegAddrBits>8)
        {
            I2CM_SendByte(iCfg,(iRegAddr&0xff00)>>8);		//���Ĵ�����ַ
            ackflag=I2CM_Wait_Ack(iCfg);       
            I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,Data[0]=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,pWrite[0],iLen);
            if(ackflag==I2CM_ACK_ERROR) return I2CM_REGH_ACKERR;
        }
        
        I2CM_SendByte(iCfg,(uint8_t)(iRegAddr&0x00ff));		//���Ĵ�����ַ
        ackflag=I2CM_Wait_Ack(iCfg);
        I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,Data[0]=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,pWrite[0],iLen);
        if(ackflag==I2CM_ACK_ERROR) return I2CM_REGL_ACKERR;
    }
	for(i=0;i<iLen;i++)
	{
		I2CM_SendByte(iCfg,pWrite[i]);	
		ackflag=I2CM_Wait_Ack(iCfg); 
		I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,Data[0]=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,pWrite[0],iLen);
        if(ackflag==I2CM_ACK_ERROR) return I2CM_WDATA_ACKERR;
	}	 										  		   	 		    	   
    I2CM_Stop(iCfg);					//ֹͣ
	I2CM_DelayTime(iCfg->Delay);
	return I2CM_EOK ;	 
}

/**
  * @brief  .I2C�����ֽں���
  * @note   .������ʼ�����źŵ�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  *			.pWrite:�������ݻ�������ʼ��ַ
  *			.iLen:���յ��ֽ���
  * @retval .��״̬��־
  */
eI2CM_Error I2CM_Read(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint16_t iRegAddr,uint8_t *iData,uint8_t iLen)
{
	uint8_t i;
	uint8_t ackflag=0;

	Init_Judge(iCfg,"READ I2CM is no init \n");    
    
	Address_Judge(iCfg,iDevAddr);

    if(iCfg->RegAddrBits>0)
    {
        I2CM_Start(iCfg);   
        I2CM_SendByte(iCfg,iDevAddr);		//���豸��ַ
        ackflag=I2CM_Wait_Ack(iCfg);
        I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,iLen);
        if(ackflag==I2CM_ACK_ERROR) return I2CM_WDEV_ACKERR;
    
        if(iCfg->RegAddrBits>8)
        {
            I2CM_SendByte(iCfg,(iRegAddr&0xff00)>>8);		//���Ĵ�����ַ
            ackflag=I2CM_Wait_Ack(iCfg);       
            I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,iLen);
            if(ackflag==I2CM_ACK_ERROR) return I2CM_REGH_ACKERR;
        }
        
        I2CM_SendByte(iCfg,(iRegAddr&0x00ff));		//���Ĵ�����ַ
        ackflag=I2CM_Wait_Ack(iCfg);
        I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,iLen);
        if(ackflag==I2CM_ACK_ERROR) return I2CM_REGL_ACKERR;
    }
	I2CM_Start(iCfg);
	I2CM_SendByte(iCfg,iDevAddr+1);		//�������ģʽ			   
	ackflag=I2CM_Wait_Ack(iCfg);
	I2CM_DEBUG(0x01,"[%s,%d]ACK=%d :Dev=0x%x,Reg=%d,iLen=%d\n",__FUNCTION__,__LINE__,ackflag,iDevAddr,iRegAddr,iLen);
    if(ackflag==I2CM_ACK_ERROR) return I2CM_RDEV_ACKERR;

	for(i=0;i<iLen-1;i++)
	{
		iData[i]=I2CM_ReadByte(iCfg,1);			//send ACK 				
	}
	iData[iLen-1]=I2CM_ReadByte(iCfg,0);		//send NACK;	
		   
    I2CM_Stop(iCfg);							//ֹͣ
	return I2CM_EOK ;	 	
}

/**
  * @brief  .I2Cдһ���ֽں���
  * @note   .������ʼ�����źŵ�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  *			.iData:д���һ���ֽ�����
  * @retval .д״̬��־
  */
eI2CM_Error I2CM_WriteOneByte(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint8_t iRegAddr,uint8_t iData)
{
	return I2CM_Write(iCfg,iDevAddr,iRegAddr,&iData,1);
}

/**
  * @brief  .I2C��һ���ֽں���
  * @note   .������ʼ�����źŵ�
  * @param  .I2CX:I2C�ӿ�(����:I2CM1,I2CM2,I2CM3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  *			.iData:�������ֽ����ڴ��еĵ�ַ
  * @retval .��״̬��־
  */
eI2CM_Error I2CM_ReadOneByte(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint8_t iRegAddr,uint8_t *iData)
{	
	return I2CM_Read(iCfg,iDevAddr,iRegAddr,iData,1);
}

/**
  * @brief  .I2Cɨ������ID
  * @note   .0~255ѭ��ɨ�裬ֻ����xsheel����
  * @param  .I2CX:I2C�ӿ�(����:1,2,3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  * @retval .void
  */
void I2CM_ScanDev(sI2CM_CfgInfo *iCfg)
{
	uint16_t i;
    uint8_t ackflag;
    uint8_t devaddr;
    uint8_t maxdevaddr;
    uint8_t add;
    
    if(iCfg->DevAddrBits==7)   //7bit��ַ 0~127
    {
        maxdevaddr=127;
        add=1;
    }
    else                  //8bitд��ַ 0~256 ż��
    {
        maxdevaddr=255;
        add=2;
    }
	for(i=0;i<=maxdevaddr;i+=add)
	{
        devaddr=i;
        Address_Judge(iCfg,devaddr);
        I2CM_Start(iCfg);   
        I2CM_SendByte(iCfg,devaddr);		//���豸��ַ
        ackflag=I2CM_Wait_Ack(iCfg);
        I2CM_Stop(iCfg);					//ֹͣ
        if(ackflag==I2CM_EOK) rt_kprintf("Device Addr=0x%02x, ACK OK\n",i);
        rt_thread_delay(MSec(10));
	}
    rt_kprintf("I2CM Scan Device Addr End !\n");
}

/**
  * @brief  .I2Cдһ���ֽں���
  * @note   .�˺���ֻ����xsheel����
  * @param  .I2CX:I2C�ӿ�(����:1,2,3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  *			.iData:д���һ���ֽ�����
  * @retval .д״̬��־
  */
void I2CM_Xwrite(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint16_t iRegAddr,uint8_t iData)
{
	eI2CM_Error err;
    err=I2CM_Write(iCfg,iDevAddr,iRegAddr,&iData,1);
    if(err!=I2CM_EOK) rt_kprintf("write error = %d\n",err);
}

/**
  * @brief  .I2C������
  * @note   .�˺���ֻ����xsheel����
  * @param  .I2CX:I2C�ӿ�(����:1,2,3)
  *         .iDevAddr:�ӻ���ַ
  *         .iRegAddr:�Ĵ�����ַ
  *			.iData:�������ֽ����ڴ��еĵ�ַ
  * @retval .��״̬��־
  */
void I2CM_Xread(sI2CM_CfgInfo *iCfg,uint8_t iDevAddr,uint16_t iRegAddr,uint8_t iLen)
{
	eI2CM_Error err;
	uint16_t i;
    uint8_t *data;
    
    if(iLen==0) return;
    data=rt_malloc(iLen);
    
	err=I2CM_Read(iCfg,iDevAddr,iRegAddr,data,iLen);
	rt_kprintf("I2CM Read Data:");
    for(i=0;i<iLen;i++)
    {
        rt_kprintf("reg=%d,0x%02x\n",iRegAddr+i,data[i]);
    }
    if(err!=I2CM_EOK) rt_kprintf("read error = %d\n",err);
    
    rt_free(data);
}

void I2CM_CheckSCL(sI2CM_CfgInfo *iCfg,uint32_t iTimes,uint16_t iDelay)
{
    uint32_t i;
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        for(i=0;i<iTimes;i++)
        {
            SCL1(iCfg);
            I2CM_DelayTime(iDelay);
            SCL0(iCfg);
            I2CM_DelayTime(iDelay);
        }
    }
    else
    {
//        for(i=0;i<iTimes;i++)
//        {
//            FSCL1(iCfg);
//            I2CM_DelayTime(iDelay);
//            FSCL0(iCfg);
//            I2CM_DelayTime(iDelay);
//        }
    }
}

void I2CM_CheckSDA(sI2CM_CfgInfo *iCfg,uint32_t iTimes,uint16_t iDelay)
{
    uint32_t i;
    if(iCfg->IOType==I2CM_IO_TYPE_MCU_IO)
    {
        for(i=0;i<iTimes;i++)
        {
            SDA1(iCfg);
            I2CM_DelayTime(iDelay);
            SDA0(iCfg);
            I2CM_DelayTime(iDelay);
        }
    }
    else
    {
//        for(i=0;i<iTimes;i++)
//        {
//            SDA1(iCfg);
//            I2CM_DelayTime(iDelay);
//            SDA0(iCfg);
//            I2CM_DelayTime(iDelay);
//        }        
    }
}

void I2CM_ShowCfgInfo(sI2CM_CfgInfo *iCfg)
{
    rt_kprintf("  IOType=%d\n",iCfg->IOType);
    rt_kprintf("  DevAddrBits=%d\n",iCfg->DevAddrBits);
    rt_kprintf("  RegAddrBits=%d\n",iCfg->RegAddrBits);
    rt_kprintf("  SCL_GPIO=0x%08x\n",iCfg->SCL_GPIO);
    rt_kprintf("  SCL_Pin=0x%x\n",iCfg->SCL_Pin);
    rt_kprintf("  SDA_GPIO=0x%08x\n",iCfg->SDA_GPIO);
    rt_kprintf("  SDA_Pin=0x%x\n",iCfg->SDA_Pin);
    rt_kprintf("  Delay=%d\n",iCfg->Delay);
    rt_kprintf("  ACKFlag=%d\n",iCfg->ACKFlag);
    rt_kprintf("  InitFlag=%d\n",iCfg->InitFlag);
    rt_kprintf("  SCL_Pin_x=%d\n",iCfg->SCL_Pin_x);
    rt_kprintf("  SDA_Pin_x=%d\n",iCfg->SDA_Pin_x);
    rt_kprintf("  FSCL_SubSite=%d\n",iCfg->FSCL_SubSite);
    rt_kprintf("  FSCL_OutReg=%d\n",iCfg->FSCL_OutReg);
    rt_kprintf("  FSCL_BitPos=%d\n",iCfg->FSCL_BitPos);
    rt_kprintf("  FSDA_SubSite=%d\n",iCfg->FSDA_SubSite);
    rt_kprintf("  FSDA_CfgReg=%d\n",iCfg->FSDA_CfgReg);
    rt_kprintf("  FSDA_OutReg=%d\n",iCfg->FSDA_OutReg);
    rt_kprintf("  FSDA_InReg=%d\n",iCfg->FSDA_InReg);
    rt_kprintf("  FSDA_BitPos=%d\n",iCfg->FSDA_BitPos);
    rt_kprintf("\nAppendix:GPIO Base Addr Info:\n");
    rt_kprintf("  GPIOA = 0x%08x\n",GPIOA_BASE);
    rt_kprintf("  GPIOB = 0x%08x\n",GPIOB_BASE);
    rt_kprintf("  GPIOC = 0x%08x\n",GPIOC_BASE);
    rt_kprintf("  GPIOD = 0x%08x\n",GPIOD_BASE);
    rt_kprintf("  GPIOE = 0x%08x\n",GPIOE_BASE);
    rt_kprintf("  GPIOF = 0x%08x\n",GPIOF_BASE);
    rt_kprintf("  GPIOH = 0x%08x\n",GPIOH_BASE);
    rt_kprintf("  GPIOI = 0x%08x\n",GPIOI_BASE);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(I2CM_ScanDev, I2CM_ScanDev(1))
FINSH_FUNCTION_EXPORT(I2CM_Xwrite, I2CM_Xwrite(1,0x5D,0x20,0x04))
FINSH_FUNCTION_EXPORT(I2CM_Xread, I2CM_Xread(1,0x5D,0x2B))
FINSH_FUNCTION_EXPORT(I2CM_CheckSCL, I2CM_CheckSCL(1,1000,1))
FINSH_FUNCTION_EXPORT(I2CM_CheckSDA, I2CM_CheckSDA(1,1000,1))
FINSH_FUNCTION_EXPORT(I2CM_ShowCfgInfo, I2CM_ShowCfgInfo())

#endif

