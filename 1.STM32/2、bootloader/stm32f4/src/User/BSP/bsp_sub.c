/*************************************************************************
 * File     : bsp_sub.c
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-10      aaron           first version
 ************************************************************************/
/*** Including ***/
#include "stm32f4xx.h"
#include <rthw.h> 
#include "rtthread.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "bsp_sub.h"
#include "finsh.h"
#include "bsp_common.h"
static uint8_t SubDebugOpt=0;
DBG_EXPORT(SubDebugOpt,sub,sub board debug option)
#define SUB_DEBUG(opt,format,...) if(opt&SubDebugOpt) {rt_kprintf("[sub:%02x] "format"",opt,##__VA_ARGS__);}
static uint8_t SubInitFlag=0;


/*** SPI3 SCK ***/
#define SPI3_SCK_GPIO           GPIOB
#define SPI3_SCK_RCC            RCC_AHB1Periph_GPIOB
#define SPI3_SCK_PIN            GPIO_Pin_3
#define SPI3_SCK_PinSource      GPIO_PinSource3

/*** SPI3 MISO ***/
#define SPI3_MISO_GPIO          GPIOB
#define SPI3_MISO_RCC           RCC_AHB1Periph_GPIOB
#define SPI3_MISO_PIN           GPIO_Pin_4
#define SPI3_MISO_PinSource     GPIO_PinSource4

/*** SPI3 MOSI ***/
#define SPI3_MOSI_GPIO          GPIOC
#define SPI3_MOSI_RCC           RCC_AHB1Periph_GPIOC
#define SPI3_MOSI_PIN           GPIO_Pin_12
#define SPI3_MOSI_PinSource     GPIO_PinSource12

/*** SPI3 NSS ***/
#define SPI3_NSS_GPIO         GPIOA
#define SPI3_NSS_RCC          RCC_AHB1Periph_GPIOA
#define SPI3_NSS_Pin          GPIO_Pin_15
#define SPI3_NSS_H            SPI3_NSS_GPIO->BSRRL=SPI3_NSS_Pin
#define SPI3_NSS_L            SPI3_NSS_GPIO->BSRRH=SPI3_NSS_Pin

/*PB7*/
#define SUB1_CS1_GPIO         GPIOB
#define SUB1_CS1_Pin          GPIO_Pin_7
#define SUB1_CS1_RCC          RCC_AHB1Periph_GPIOB
#define SUB1_CS1_H            SUB1_CS1_GPIO->BSRRL=SUB1_CS1_Pin
#define SUB1_CS1_L            SUB1_CS1_GPIO->BSRRH=SUB1_CS1_Pin

/*PA8*/
#define SUB1_CS2_GPIO         GPIOA
#define SUB1_CS2_Pin          GPIO_Pin_8
#define SUB1_CS2_RCC          RCC_AHB1Periph_GPIOA
#define SUB1_CS2_H            SUB1_CS2_GPIO->BSRRL=SUB1_CS2_Pin
#define SUB1_CS2_L            SUB1_CS2_GPIO->BSRRH=SUB1_CS2_Pin

/*PD7*/
#define SUB1_CS3_GPIO         GPIOD
#define SUB1_CS3_Pin          GPIO_Pin_7
#define SUB1_CS3_RCC          RCC_AHB1Periph_GPIOD
#define SUB1_CS3_H            SUB1_CS3_GPIO->BSRRL=SUB1_CS3_Pin
#define SUB1_CS3_L            SUB1_CS3_GPIO->BSRRH=SUB1_CS3_Pin

/*PF8*/
#define SUB2_CS1_GPIO         GPIOF
#define SUB2_CS1_Pin          GPIO_Pin_8
#define SUB2_CS1_RCC          RCC_AHB1Periph_GPIOF
#define SUB2_CS1_H            SUB2_CS1_GPIO->BSRRL=SUB2_CS1_Pin
#define SUB2_CS1_L            SUB2_CS1_GPIO->BSRRH=SUB2_CS1_Pin

/*PF6*/
#define SUB2_CS2_GPIO         GPIOF
#define SUB2_CS2_Pin          GPIO_Pin_6
#define SUB2_CS2_RCC          RCC_AHB1Periph_GPIOF
#define SUB2_CS2_H            SUB2_CS2_GPIO->BSRRL=SUB2_CS2_Pin
#define SUB2_CS2_L            SUB2_CS2_GPIO->BSRRH=SUB2_CS2_Pin

/*PF5*/
#define SUB2_CS3_GPIO         GPIOF
#define SUB2_CS3_Pin          GPIO_Pin_5
#define SUB2_CS3_RCC          RCC_AHB1Periph_GPIOF
#define SUB2_CS3_H            SUB2_CS3_GPIO->BSRRL=SUB2_CS3_Pin
#define SUB2_CS3_L            SUB2_CS3_GPIO->BSRRH=SUB2_CS3_Pin



#define Enable_SUB1_1   {SUB1_CS1_L;SUB1_CS2_L;SUB1_CS3_L;SPI3_NSS_L;}
#define Enable_SUB1_2   {SUB1_CS1_H;SUB1_CS2_L;SUB1_CS3_L;SPI3_NSS_L;}
#define Enable_SUB1_3   {SUB1_CS1_L;SUB1_CS2_H;SUB1_CS3_L;SPI3_NSS_L;}
#define Enable_SUB1_4   {SUB1_CS1_H;SUB1_CS2_H;SUB1_CS3_L;SPI3_NSS_L;}
#define Enable_SUB1_5   {SUB1_CS1_L;SUB1_CS2_L;SUB1_CS3_H;SPI3_NSS_L;}
#define Enable_SUB1_6   {SUB1_CS1_H;SUB1_CS2_L;SUB1_CS3_H;SPI3_NSS_L;}
#define Enable_SUB1_7   {SUB1_CS1_L;SUB1_CS2_H;SUB1_CS3_H;SPI3_NSS_L;}
#define Disable_SUB1    {SUB1_CS1_H;SUB1_CS2_H;SUB1_CS3_H;SPI3_NSS_H;}

#define Enable_SUB2_1   {SUB2_CS1_L;SUB2_CS2_L;SUB2_CS3_L;SPI3_NSS_L;}
#define Enable_SUB2_2   {SUB2_CS1_H;SUB2_CS2_L;SUB2_CS3_L;SPI3_NSS_L;}
#define Enable_SUB2_3   {SUB2_CS1_L;SUB2_CS2_H;SUB2_CS3_L;SPI3_NSS_L;}
#define Enable_SUB2_4   {SUB2_CS1_H;SUB2_CS2_H;SUB2_CS3_L;SPI3_NSS_L;}
#define Enable_SUB2_5   {SUB2_CS1_L;SUB2_CS2_L;SUB2_CS3_H;SPI3_NSS_L;}
#define Enable_SUB2_6   {SUB2_CS1_H;SUB2_CS2_L;SUB2_CS3_H;SPI3_NSS_L;}
#define Enable_SUB2_7   {SUB2_CS1_L;SUB2_CS2_H;SUB2_CS3_H;SPI3_NSS_L;}
#define Disable_SUB2    {SUB2_CS1_H;SUB2_CS2_H;SUB2_CS3_H;SPI3_NSS_H;}


#define FPGA_REG_WIDTH      6       //FPGA的寄存器buffer位宽
#define FPGA_RD_CMD         0       //FPGA寄存器buffer读命令
#define FPGA_WR_CMD         1       //FPGA寄存器buffer写命令

sSubBoardInfo SubBoardInfo[SUB_BOARD_NUM];
static uint8_t SubReg[SUB_BOARD_NUM][FSUB_REG_MAX_NUM<<1];/*存放每个子板中64个寄存器的值：在本数组中偶数位置存放寄存器值，奇数位置存放TRAP_VALUE*/
static const char SubBoardName[SUB_BOARD_TYPE_NUM+1][24]=
{
    "Switch Board A",
    "IO Board",
    "Mic Board",
    "Switch Board B",
    "IC Board",
    "LeakCurrent Board",
};

/*--------------------------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------------  静态函数区  -----------------------------------------------------*
 *--------------------------------------------------------------------------------------------------------------------------*/

static void NSS_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(SUB1_CS1_RCC | SUB1_CS2_RCC | SUB1_CS3_RCC  |  SUB2_CS1_RCC | SUB2_CS2_RCC | SUB2_CS3_RCC | SPI3_NSS_RCC, ENABLE);    

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    
    GPIO_InitStructure.GPIO_Pin = SUB1_CS1_Pin;
    GPIO_Init(SUB1_CS1_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = SUB1_CS2_Pin;
    GPIO_Init(SUB1_CS2_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = SUB1_CS3_Pin;
    GPIO_Init(SUB1_CS3_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = SUB2_CS1_Pin;
    GPIO_Init(SUB2_CS1_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = SUB2_CS2_Pin;
    GPIO_Init(SUB2_CS2_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = SUB2_CS3_Pin;
    GPIO_Init(SUB2_CS3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = SPI3_NSS_Pin;
    GPIO_Init(SPI3_NSS_GPIO, &GPIO_InitStructure);
    
    Disable_SUB1;
    Disable_SUB2;
}

static void SPI3_GPIO_Config(void)
{
    GPIO_InitTypeDef    GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(SPI3_SCK_RCC | SPI3_MISO_RCC | SPI3_MOSI_RCC, ENABLE);       // | SPI3_CS_RCC
    
    GPIO_PinAFConfig(SPI3_SCK_GPIO,SPI3_SCK_PinSource,GPIO_AF_SPI3);
    GPIO_PinAFConfig(SPI3_MISO_GPIO,SPI3_MISO_PinSource,GPIO_AF_SPI3);
    GPIO_PinAFConfig(SPI3_MOSI_GPIO,SPI3_MOSI_PinSource,GPIO_AF_SPI3);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;                         //CLK/MISO/MOSI 三个GPIO的初始化
    
    GPIO_InitStructure.GPIO_Pin   = SPI3_SCK_PIN;
    GPIO_Init(SPI3_SCK_GPIO, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = SPI3_MISO_PIN;
    GPIO_Init(SPI3_MISO_GPIO, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = SPI3_MOSI_PIN;
    GPIO_Init(SPI3_MOSI_GPIO, &GPIO_InitStructure);
}

static void SPI3_Init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    
    SPI3_GPIO_Config(); 
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI3, &SPI_InitStructure);
    SPI_Cmd(SPI3, ENABLE);
}

#define FPGA_SendData(iCmd,iSubSite,iData)                      \
{                                                               \
    switch(iSubSite)                                            \
    {                                                           \
        case SUB1_1:                                            \
            Enable_SUB1_1;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB1_2:                                            \
            Enable_SUB1_2;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB1_3:                                            \
            Enable_SUB1_3;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB1_4:                                            \
            Enable_SUB1_4;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB1_5:                                            \
            Enable_SUB1_5;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB1_6:                                            \
            Enable_SUB1_6;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB1_7:                                            \
            Enable_SUB1_7;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB1;                                       \
            break;                                              \
        case SUB2_1:                                            \
            Enable_SUB2_1;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        case SUB2_2:                                            \
            Enable_SUB2_2;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        case SUB2_3:                                            \
            Enable_SUB2_3;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        case SUB2_4:                                            \
            Enable_SUB2_4;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        case SUB2_5:                                            \
            Enable_SUB2_5;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        case SUB2_6:                                            \
            Enable_SUB2_6;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        case SUB2_7:                                            \
            Enable_SUB2_7;                                      \
            while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      \
            SPI3->DR = iCmd;                                    \
            while((SPI3->SR & SPI_I2S_FLAG_RXNE) == RESET);     \
            iData=SPI3->DR;                                     \
            Disable_SUB2;                                       \
            break;                                              \
        default:                                                \
            rt_kprintf("sub board %d is not exist!\n",iSubSite); \
            break;                                              \
    }                                                           \
}
/*--------------------------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------------  全局函数区  -----------------------------------------------------*
 *--------------------------------------------------------------------------------------------------------------------------*/

void SUB_Init(void)
{  
    uint8_t i,j;
    uint8_t regvalue;
    uint8_t type;
    uint8_t vesion;
    
    PRINT_INIT_START();
    
    NSS_GPIO_Configuration();
    SPI3_Init();
    rt_thread_delay(MSec(500));
    
    memset(SubReg,0,sizeof(SubReg));

    /*对SubReg设置TrapValue*/
    for(i=0;i<SUB_BOARD_NUM;i++)
    {
        for(j=1;j<(FSUB_REG_MAX_NUM<<1);j+=2) SubReg[i][j]=TRAP_VALUE8;
    }

    SubInitFlag=1; //接下来的代码需要操作本文件中的函数，因此必须在此处将初始化标志置为1

    /*读取每个子板的初始配置值*/
    for(i=0;i<SUB_BOARD_NUM;i++)
    {
        for(j=0;j<FSUB_REG_MAX_NUM;j++) SUB_ReadByte(j,(eSubSite)i); 
    }
    
    /*获取板卡信息*/
    rt_kprintf("---------------------------Sub Board Info---------------------\n");
    rt_kprintf("Site    Type    Name                    Version    Seq \n");
    for(i=0;i<SUB_BOARD_NUM;i++)
    {
        regvalue=SUB_ReadByte(FSUB_REV_ADDR,(eSubSite)i);
        type=regvalue>>3;
        vesion=regvalue&0x07;
        
        
        SubBoardInfo[i].Type=type;
        SubBoardInfo[i].Seq=0;
        if(type<SUB_BOARD_TYPE_NUM)
        {
            SubBoardInfo[i].Name=SubBoardName[type];
            SubBoardInfo[i].Version=vesion;
            if(i==0) SubBoardInfo[i].Seq=0;
            else
            {
                for(j=0;j<i;j++)
                {
                    if(SubBoardInfo[j].Type==type) 
                    {
                        SubBoardInfo[i].Seq=SubBoardInfo[j].Seq+1;
                    }
                }
            }
            rt_kprintf("%02d      %02d      %-20s    %d          %d\n",i+1,type,SubBoardName[type],vesion,SubBoardInfo[i].Seq);
        }
        else
        {
            SubBoardInfo[i].Name=SubBoardName[SUB_BOARD_TYPE_NUM];
            SubBoardInfo[i].Version=0;
            SubBoardInfo[i].Seq=0;
            rt_kprintf("%02d      %02d      %-20s\n",i+1,type,SubBoardName[SUB_BOARD_TYPE_NUM]);
        }
    }
    PRINT_INIT_OK();
}

/*通过板卡类型及序号获取板卡所在槽位号*/
eSubSite SUB_GetSiteFromSeq(eSubBoardType iBoardType,uint8_t iSeq)
{
    uint8_t i;
    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    
    for(i=0;i<SUB_BOARD_NUM;i++)
    {
        if(iBoardType==SubBoardInfo[i].Type)
        {
            if(iSeq == SubBoardInfo[i].Seq) return (eSubSite)i;
        }
    }
    rt_kprintf("%s,Can not find SubSite (Seq=%d)\n",__FUNCTION__,iSeq);
    return (eSubSite)i;
}

/*一次性操作8个IO*/
void SUB_WriteByte(uint8_t iRegAddr, uint8_t iData, eSubSite iSubSite)
{
    rt_base_t level;
    uint16_t cmd;
    uint8_t recv;

    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return;}

    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");
    
    level=rt_hw_interrupt_disable();
    SubReg[iSubSite][iRegAddr<<1]=iData;
    cmd = (((FPGA_WR_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8) | SubReg[iSubSite][iRegAddr<<1];
    FPGA_SendData(cmd,iSubSite,recv);
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,site=%d,data=%x,recv=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iSubSite,iData,recv);
    return;
}

uint8_t  SUB_ReadByte(uint8_t iRegAddr, eSubSite iSubSite)
{
    rt_base_t level;
    uint8_t value=0;
    uint16_t cmd;

    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return 0;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return 0;}

    cmd = ((FPGA_RD_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8;
    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");
    
    level=rt_hw_interrupt_disable();
    FPGA_SendData(cmd,iSubSite,value);    
    SubReg[iSubSite][iRegAddr<<1]=value;
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,site=%d,value=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iSubSite,value);
    return value;
}

/*将指定IO寄存器位设置为1*/
void SUB_SetBit(uint8_t iRegAddr, uint8_t iBit, eSubSite iSubSite)
{
    rt_base_t level;
    uint16_t cmd;
    uint8_t recv;
    
    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return;}

    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");
    
    level=rt_hw_interrupt_disable();
    BitSet(SubReg[iSubSite][iRegAddr<<1],iBit);//将本地记录的寄存器设置为相应状态
    cmd = (((FPGA_WR_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8) | SubReg[iSubSite][iRegAddr<<1];
    FPGA_SendData(cmd,iSubSite,recv);
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,bit=%d,site=%d,recv=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iBit,iSubSite,recv);
    return ;
}

/*将指定IO寄存器位设置为0*/
void SUB_ResetBit(uint8_t iRegAddr, uint8_t iBit, eSubSite iSubSite)
{
    rt_base_t level;
    uint16_t cmd;
    uint8_t recv;
    
    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return;}

    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");

    level=rt_hw_interrupt_disable();
    BitClear(SubReg[iSubSite][iRegAddr<<1],iBit);//将本地记录的寄存器设置为相应状态
    cmd = (((FPGA_WR_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8) | SubReg[iSubSite][iRegAddr<<1];   
    FPGA_SendData(cmd,iSubSite,recv);  
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,bit=%d,site=%d,recv=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iBit,iSubSite,recv);
    return;        
}

void SUB_SetBits(uint8_t iRegAddr, uint8_t iBits, eSubSite iSubSite)
{
    rt_base_t level;
    uint16_t cmd;
    uint8_t recv;
    
    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return;}

    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");
    
    level=rt_hw_interrupt_disable();
    SubReg[iSubSite][iRegAddr<<1] |= iBits;//将本地记录的寄存器设置为相应状态
    cmd = (((FPGA_WR_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8) | SubReg[iSubSite][iRegAddr<<1];
    FPGA_SendData(cmd,iSubSite,recv);
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,bits=%d,site=%d,recv=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iBits,iSubSite,recv);
    return ;
}

void SUB_ResetBits(uint8_t iRegAddr, uint8_t iBits, eSubSite iSubSite)
{
    rt_base_t level;
    uint16_t cmd;
    uint8_t recv;
    
    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return;}

    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");
    
    level=rt_hw_interrupt_disable();
    SubReg[iSubSite][iRegAddr<<1] &= iBits;//将本地记录的寄存器设置为相应状态
    cmd = (((FPGA_WR_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8) | SubReg[iSubSite][iRegAddr<<1];
    FPGA_SendData(cmd,iSubSite,recv);
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,bits=%d,site=%d,recv=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iBits,iSubSite,recv);
    return ;
}


uint8_t SUB_GetBit(uint8_t iRegAddr, uint8_t iBit, eSubSite iSubSite)
{
    uint16_t cmd;
    uint8_t value=0;
    rt_base_t level;
    
    FWAssertError(SubInitFlag,"Please Call Function SUB_Init() to Init SubBoard");
    if(iRegAddr>=FSUB_REG_MAX_NUM) {rt_kprintf("error : %s :iRegAddr(%d)>=%d\n",__FUNCTION__,iRegAddr,FSUB_REG_MAX_NUM); return 0;}
    if(iSubSite>=SUB_BOARD_NUM)    {rt_kprintf("error : %s :iSubSite(%d)>=%d\n",__FUNCTION__,iSubSite,SUB_BOARD_NUM);return 0;}
    
    cmd = (((FPGA_RD_CMD << FPGA_REG_WIDTH) | iRegAddr) << 8);   
    TRAP_CHECK8(&SubReg[iSubSite][(iRegAddr<<1)+1],"SubReg");
    
    level=rt_hw_interrupt_disable();
    FPGA_SendData(cmd,iSubSite,value);    
    SubReg[iSubSite][iRegAddr<<1]=value;
    rt_hw_interrupt_enable(level);
    
    SUB_DEBUG(0x01,"[%s,%d]:cmd=%x,reg=%d,bit=%d,site=%d,value=%x\n",__FUNCTION__,__LINE__,cmd,iRegAddr,iBit,iSubSite,value);
    return BitGet(value, iBit);
}


void SPI_Test(void)
{
    uint16_t iCmd=0;
    uint8_t i;
    TIME_MEASURE_VAR_DEF;
    
    TIME_MEASURE_START;
    for(i=0;i<100;i++)
    {   
        while((SPI3->SR & SPI_I2S_FLAG_TXE) == RESET);      
        SPI3->DR = iCmd;                                    
    }    
    TIME_MEASURE_STOP;    
}
void SPI_IO(uint8_t iSubState)
{
    Disable_SUB1;
    Disable_SUB2;
    
    if(iSubState==SUB1_1) Enable_SUB1_1;
    if(iSubState==SUB1_2) Enable_SUB1_2;
    if(iSubState==SUB1_3) Enable_SUB1_3;
    if(iSubState==SUB1_4) Enable_SUB1_4;
    if(iSubState==SUB1_5) Enable_SUB1_5;
    if(iSubState==SUB1_6) Enable_SUB1_6;
    if(iSubState==SUB1_7) Enable_SUB1_7;
    
    if(iSubState==SUB2_1) Enable_SUB2_1;
    if(iSubState==SUB2_2) Enable_SUB2_2;
    if(iSubState==SUB2_3) Enable_SUB2_3;
    if(iSubState==SUB2_4) Enable_SUB2_4;
    if(iSubState==SUB2_5) Enable_SUB2_5;
    if(iSubState==SUB2_6) Enable_SUB2_6;
    if(iSubState==SUB2_7) Enable_SUB2_7;
}

void RegSub(uint8_t iSubSite,uint8_t iType)
{
    uint8_t sub=0,i=0,reg=0,data=0;
    if(iSubSite>=SUB_BOARD_NUM)
    {
        for(sub=0;sub<SUB_BOARD_NUM;sub++)
        {
            rt_kprintf("\n------------------------------------SubSite %d------------------------------------\n",sub+1);
            for(reg=0;reg<FSUB_REG_MAX_NUM;reg++)
            {
                if(reg==0) rt_kprintf("******Reg00~07 : Output Register******\n");
                if(reg==8) rt_kprintf("\n******Reg08~15 : Output Enable Register******\n");
                if(reg==16) rt_kprintf("\n******Reg16~23 : Input Register******\n");
                if(reg==24) rt_kprintf("\n******Reg24~31 : External IO Register******\n");
                if(reg==32) rt_kprintf("\n******Reg32~63 : Function Register******\n");
                if(iType==0) //读FPGA的数据
                {
                    data=SUB_ReadByte(reg,(eSubSite)sub);
                }
                else //读本地缓存的寄存器
                {
                    data=SubReg[iSubSite][(reg<<1)];
                }
                rt_kprintf("Reg%02d=0x%02X(0b'",reg,data);
                for(i=0;i<8;i++) 
                {
                    if(i==4) rt_kprintf(" ");
                    rt_kprintf("%d",BitGet(data,7-i));
                }
                rt_kprintf(")  ");
                if(reg%4==3) rt_kprintf("\n");
            }
        }
    }
    else
    {
        sub=iSubSite;
        rt_kprintf("\n------------------------------------SubSite %d------------------------------------\n",sub+1);
        for(reg=0;reg<FSUB_REG_MAX_NUM;reg++)
        {
            if(reg==0) rt_kprintf("******Reg00~07 : Output Register******\n");
            if(reg==8) rt_kprintf("\n******Reg08~15 : Output Enable Register******\n");
            if(reg==16) rt_kprintf("\n******Reg16~23 : Input Register******\n");
            if(reg==24) rt_kprintf("\n******Reg24~31 : External IO Register******\n");
            if(reg==32) rt_kprintf("\n******Reg32~63 : Function Register******\n");
            if(iType==0) //读FPGA的数据
            {
                data=SUB_ReadByte(reg,(eSubSite)sub);
            }
            else //读本地缓存的寄存器
            {
                data=SubReg[iSubSite][(reg<<1)];
            }
            rt_kprintf("Reg%02d=0x%02X(0b'",reg,data);
            for(i=0;i<8;i++) 
            {
                if(i==4) rt_kprintf(" ");
                rt_kprintf("%d",BitGet(data,7-i));
            }
            rt_kprintf(")  ");
            if(reg%4==3) rt_kprintf("\n");
        }        
    }
}


FINSH_FUNCTION_EXPORT(SPI_Test, .)
FINSH_FUNCTION_EXPORT(SPI_IO, .)
FINSH_FUNCTION_EXPORT(RegSub, .)
FINSH_FUNCTION_EXPORT(SUB_WriteByte, .)
