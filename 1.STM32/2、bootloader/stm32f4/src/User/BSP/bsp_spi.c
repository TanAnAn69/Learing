/*************************************************************************
 * File     : bsp_spi.c
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-10      aaron           first version
 ************************************************************************/
#include "bsp_common.h"
#include "bsp_spi.h" 
#include "finsh.h"
 
 #define SPI1_DEBUG(opt,format,...) if(opt&SPI1DebugOpt) {rt_kprintf("[spi1:%02x] "format"",opt,##__VA_ARGS__);}
 #define SPI2_DEBUG(opt,format,...) if(opt&SPI2DebugOpt) {rt_kprintf("[spi2:%02x] "format"",opt,##__VA_ARGS__);}

/*** SPI1 SCK ***/
#define SPI1_SCK_GPIO           GPIOA
#define SPI1_SCK_PIN            GPIO_Pin_5
#define SPI1_SCK_RCC            RCC_AHB1Periph_GPIOA
#define SPI1_SCK_PinSource      GPIO_PinSource5

/*** SPI1 MISO ***/
#define SPI1_MISO_GPIO          GPIOA
#define SPI1_MISO_PIN           GPIO_Pin_6
#define SPI1_MISO_RCC           RCC_AHB1Periph_GPIOA
#define SPI1_MISO_PinSource     GPIO_PinSource6

/*** SPI1 MOSI ***/
#define SPI1_MOSI_GPIO          GPIOB
#define SPI1_MOSI_PIN           GPIO_Pin_5
#define SPI1_MOSI_RCC           RCC_AHB1Periph_GPIOB
#define SPI1_MOSI_PinSource     GPIO_PinSource5



/*** SPI2 SCK ***/
#define SPI2_SCK_GPIO           GPIOB
#define SPI2_SCK_PIN            GPIO_Pin_13
#define SPI2_SCK_RCC            RCC_AHB1Periph_GPIOB
#define SPI2_SCK_PinSource      GPIO_PinSource13

/*** SPI2 MISO ***/
#define SPI2_MISO_GPIO          GPIOB
#define SPI2_MISO_PIN           GPIO_Pin_14
#define SPI2_MISO_RCC           RCC_AHB1Periph_GPIOB
#define SPI2_MISO_PinSource     GPIO_PinSource14

/*** SPI2 MOSI ***/
#define SPI2_MOSI_GPIO          GPIOB
#define SPI2_MOSI_PIN           GPIO_Pin_15
#define SPI2_MOSI_RCC           RCC_AHB1Periph_GPIOB
#define SPI2_MOSI_PinSource     GPIO_PinSource15

/*SPI1变量定义*/
static struct rt_semaphore  SPI1SafeSem; //SPI1通信信息量，在使用SPI1通信前需要获取该信号量
static SPI_InitTypeDef *SPI1CurInitStruct=RT_NULL; //当前所用的SPI的初始化信息
static void (*SPI1CurNssCmd)(FunctionalState NewState)=RT_NULL;//当前所用的SPI的NSS函数
static uint8_t SPI1DebugOpt=0;
DBG_EXPORT(SPI1DebugOpt,spi1,spi1 debug option)


/*SPI2变量定义*/
static struct rt_semaphore  SPI2SafeSem; //SPI2通信信息量，在使用SPI2通信前需要获取该信号量
static SPI_InitTypeDef *SPI2CurInitStruct=RT_NULL; //当前所用的SPI的初始化信息
static void (*SPI2CurNssCmd)(FunctionalState NewState)=RT_NULL;//当前所用的SPI的NSS函数
static uint8_t SPI2DebugOpt=0;
DBG_EXPORT(SPI2DebugOpt,spi2,spi2 debug option)


static void SPI1_GPIO_Init(void)
{
    static uint8_t flag=0;
    if(flag==0)
    {
        rt_err_t err;
        GPIO_InitTypeDef    GPIO_InitStructure;
    
        PRINT_INIT_START();
        RCC_AHB1PeriphClockCmd(SPI1_SCK_RCC | SPI1_MISO_RCC | SPI1_MOSI_RCC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        
        GPIO_PinAFConfig(SPI1_SCK_GPIO,SPI1_SCK_PinSource,GPIO_AF_SPI1);
        GPIO_PinAFConfig(SPI1_MISO_GPIO,SPI1_MISO_PinSource,GPIO_AF_SPI1);
        GPIO_PinAFConfig(SPI1_MOSI_GPIO,SPI1_MOSI_PinSource,GPIO_AF_SPI1);      //复用为SPI1
        
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;                         //CLK/MISO/MOSI 三个GPIO的初始化
        
        GPIO_InitStructure.GPIO_Pin   = SPI1_SCK_PIN;
        GPIO_Init(SPI1_SCK_GPIO, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin   = SPI1_MISO_PIN;
        GPIO_Init(SPI1_MISO_GPIO, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin   = SPI1_MOSI_PIN;
        GPIO_Init(SPI1_MOSI_GPIO, &GPIO_InitStructure);
        
        err=rt_sem_init(&SPI1SafeSem, "SPI1SafeSem", 1,RT_IPC_FLAG_FIFO);
        RT_ASSERT(err == RT_EOK);
        flag=1;
        PRINT_INIT_OK();
    }
}


uint16_t SPI1_SendData(SPI_InitTypeDef *iSPI1InitStruct,void (*iNssCmd)(FunctionalState NewState),uint16_t iSendData)
{
    uint16_t recvdata;
    
    SPI1_GPIO_Init();
    
    FWAssertError(iSPI1InitStruct!=RT_NULL,"%s","iSPI1InitStruct not init!");
    
    rt_sem_take(&SPI1SafeSem,RT_WAITING_FOREVER);
    SPI1_DEBUG(0x08,"SPI1SafeSem take ok!\n");
    
    /*更改配置*/
    if(iSPI1InitStruct != SPI1CurInitStruct)
    {
        SPI_Cmd(SPI1,DISABLE);
        SPI_Init(SPI1,iSPI1InitStruct);
        SPI_Cmd(SPI1,ENABLE);
        SPI1CurInitStruct=iSPI1InitStruct;
        SPI1CurNssCmd=iNssCmd;
        SPI1_DEBUG(0x01,"SPI1InitStruct(0x%x) Init end !\n",iSPI1InitStruct);
    }
    
    /*发送数据*/
    if(iNssCmd!=RT_NULL) SPI1CurNssCmd(ENABLE);
    while((SPI1->SR & SPI_I2S_FLAG_TXE) == RESET);      
    SPI1->DR = iSendData;                                
    SPI1_DEBUG(0x02,"SPI1 Send data ok , data=%d \n",iSendData);
    
    /*获取数据*/
    while((SPI1->SR & SPI_I2S_FLAG_RXNE) == RESET);     
    recvdata=SPI1->DR;                                     
    SPI1_DEBUG(0x04,"SPI1 recv data ok , data=%d \n",recvdata);
    if(iNssCmd!=RT_NULL) SPI1CurNssCmd(DISABLE);
    
    rt_sem_release(&SPI1SafeSem);
    
    return recvdata;
}


static void SPI2_GPIO_Init(void)
{
    static uint8_t flag=0;
    if(flag==0)
    {
        rt_err_t err;
        GPIO_InitTypeDef    GPIO_InitStructure;
        
        RCC_AHB1PeriphClockCmd(SPI2_SCK_RCC | SPI2_MISO_RCC | SPI2_MOSI_RCC, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        
        GPIO_PinAFConfig(SPI2_SCK_GPIO,SPI2_SCK_PinSource,GPIO_AF_SPI2);
        GPIO_PinAFConfig(SPI2_MISO_GPIO,SPI2_MISO_PinSource,GPIO_AF_SPI2);
        GPIO_PinAFConfig(SPI2_MOSI_GPIO,SPI2_MOSI_PinSource,GPIO_AF_SPI2);      //复用为SPI2
        
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;                         //CLK/MISO/MOSI 三个GPIO的初始化
        
        GPIO_InitStructure.GPIO_Pin   = SPI2_SCK_PIN;
        GPIO_Init(SPI2_SCK_GPIO, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin   = SPI2_MISO_PIN;
        GPIO_Init(SPI2_MISO_GPIO, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin   = SPI2_MOSI_PIN;
        GPIO_Init(SPI2_MOSI_GPIO, &GPIO_InitStructure);

        err=rt_sem_init(&SPI2SafeSem, "SPI2SafeSem", 1,RT_IPC_FLAG_FIFO);
        RT_ASSERT(err == RT_EOK);
        rt_kprintf("SPI2 Init end !\n");
        flag=1;
    }
}

uint16_t SPI2_SendData(SPI_InitTypeDef *iSPI2InitStruct,void (*iNssCmd)(FunctionalState NewState),uint16_t iSendData)
{
    uint16_t recvdata;

    SPI2_GPIO_Init();
    
    FWAssertError(iSPI2InitStruct!=RT_NULL,"%s","iSPI2InitStruct is RT_NULL!");
    
    rt_sem_take(&SPI2SafeSem,RT_WAITING_FOREVER);
    SPI2_DEBUG(0x08,"SPI2SafeSem take ok!\n");
    
    /*更改配置*/
    if(iSPI2InitStruct != SPI2CurInitStruct)
    {
        SPI_Cmd(SPI2,DISABLE);
        SPI_Init(SPI2,iSPI2InitStruct);
        SPI_Cmd(SPI2,ENABLE);
        SPI2CurInitStruct=iSPI2InitStruct;
        SPI2CurNssCmd=iNssCmd;
        SPI2_DEBUG(0x01,"SPI2InitStruct(0x%x) Init end !\n",iSPI2InitStruct);
    }
    
    /*发送数据*/
    if(iNssCmd!=RT_NULL) SPI2CurNssCmd(ENABLE);
    while((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET);      
    SPI2->DR = iSendData;                                
    SPI2_DEBUG(0x02,"SPI2 Send data ok , data=%d \n",iSendData);
    
    /*获取数据*/
    while((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);     
    recvdata=SPI2->DR;                                     
    SPI2_DEBUG(0x04,"SPI2 recv data ok , data=%d \n",recvdata);
    if(iNssCmd!=RT_NULL) SPI2CurNssCmd(DISABLE);
    
    rt_sem_release(&SPI2SafeSem);
    
    return recvdata;
}

