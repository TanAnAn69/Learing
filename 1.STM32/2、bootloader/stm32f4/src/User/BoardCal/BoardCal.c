/*
 * 缩略词含义：
 * BC_:board calibration （板子校准）
 */
#include "stdtest_common.h"
extern void SBC_ResCalInit(void);
extern void SBC_ResCal(void);
extern void SBC_ResSelfTest(void);
extern void SBC_Res4WCalInit(void);
extern void SBC_Res4WCal(void);
extern void SBC_Res4WSelfTest(void); 
extern void SBC_CapCal(void);



#define BC_TYPE_CAL 0  //板卡校验
#define BC_TYPE_SELF_TEST 1  //办卡自检 
static uint8_t BC_Type=0; //板卡校验类型

static struct rt_semaphore BC_StartSem;//校验开始信号量
uint8_t BC_PauseFlag=BC_CONTINE; //暂停标志位  1：暂停运行 0：继续运行 
static uint8_t BC_DebugFlag=0;//是否进入调试模式 1：进入调试模式  0：正常校验模式

uint16_t BC_EepromAddr;
uint16_t BC_EepromSize;

/*暂停运行命令*/
void BC_PauseCmd(uint8_t iCmd)
{
	BC_PauseFlag=iCmd;	
	while(1)
	{
		if(BC_PauseFlag==BC_CONTINE) break;
		rt_thread_delay(MSec(100));
	}
}

/*用于调试时暂停执行*/
void BC_DebugEx(const char* iFile,uint16_t iLine)
{
    if(BC_DebugFlag==BC_PAUSE)
    {
        rt_kprintf("调试暂停:%s (file=%s,line=%d)\n",BC_CONTINE_TIPS,iFile,iLine);
        BC_PauseCmd(BC_PAUSE);  
    }
    else
    {
        BC_PauseCmd(BC_CONTINE);
    }        
}

FINSH_TYPE BC_DebugCmd(uint8_t iCmd)
{
    BC_DebugFlag=iCmd;    
} 

/*板卡校验*/
static void BC_BoardCal(void)
{
	MBST_Power();
	MBST_Res();
    MBST_ADCCal();
    SBC_ResCal();
    SBC_Res4WCal();
//    SBC_CapCal();
    
    rt_kprintf("\n\n******************板卡校验完成*************************\n");
}

/*板卡自检*/
static void BC_BoardSelfTest(void)
{
    SBC_ResSelfTest(); 
    SBC_Res4WSelfTest();
//    SBC_CapSelfTest();
    rt_kprintf("\n\n******************板卡自检完成*************************\n");
}

/*板卡校验初始化，所有板卡校验的初始化函数都放在此函数中*/
void BC_BoardCalInit(uint16_t iAddr,uint16_t iSize)
{
	rt_err_t result;
    PRINT_INIT_START();
    
    BC_EepromAddr=iAddr;
    BC_EepromSize=iSize;
    
    result = rt_sem_init(&BC_StartSem,"BCStarSem",0,RT_IPC_FLAG_FIFO);
	if(result != RT_EOK) rt_kprintf("Keysem init fail !\n");
    
    SBC_ResCalInit();
    SBC_Res4WCalInit();
//    SBC_CapCalInit();
    PRINT_INIT_OK();
}

/*板卡校验开始
 *uint8_t iType：0：板卡校验   1：板卡自检 255：打印帮助信息
 */
static void BCStart(uint8_t iType)
{
    if(iType==0xff)
    {
        rt_kprintf("BCStart(0) : 开始板卡校验\n");
        rt_kprintf("BCStart(1) : 开始板卡自检\n");
        return;
    }
    BC_Type=iType;
    rt_sem_release(&BC_StartSem);
}

void board_cal_thread_entry(void* parameter)
{
    while(1)
    {
        rt_sem_take(&BC_StartSem,RT_WAITING_FOREVER);
        
        if(BC_Type==BC_TYPE_CAL) BC_BoardCal();
        if(BC_Type==BC_TYPE_SELF_TEST) BC_BoardSelfTest();
        
        while(rt_sem_trytake(&BC_StartSem)==RT_EOK);
    }
}

FINSH_FUNCTION_EXPORT(BC_PauseCmd,BC_PauseCmd(0))
FINSH_FUNCTION_EXPORT(BCStart,BCStart(255))
FINSH_FUNCTION_EXPORT(BC_DebugCmd,BC_DebugCmd(1))

