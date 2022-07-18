/*
 * ���Դʺ��壺
 * BC_:board calibration ������У׼��
 */
#include "stdtest_common.h"
extern void SBC_ResCalInit(void);
extern void SBC_ResCal(void);
extern void SBC_ResSelfTest(void);
extern void SBC_Res4WCalInit(void);
extern void SBC_Res4WCal(void);
extern void SBC_Res4WSelfTest(void); 
extern void SBC_CapCal(void);



#define BC_TYPE_CAL 0  //�忨У��
#define BC_TYPE_SELF_TEST 1  //�쿨�Լ� 
static uint8_t BC_Type=0; //�忨У������

static struct rt_semaphore BC_StartSem;//У�鿪ʼ�ź���
uint8_t BC_PauseFlag=BC_CONTINE; //��ͣ��־λ  1����ͣ���� 0���������� 
static uint8_t BC_DebugFlag=0;//�Ƿ�������ģʽ 1���������ģʽ  0������У��ģʽ

uint16_t BC_EepromAddr;
uint16_t BC_EepromSize;

/*��ͣ��������*/
void BC_PauseCmd(uint8_t iCmd)
{
	BC_PauseFlag=iCmd;	
	while(1)
	{
		if(BC_PauseFlag==BC_CONTINE) break;
		rt_thread_delay(MSec(100));
	}
}

/*���ڵ���ʱ��ִͣ��*/
void BC_DebugEx(const char* iFile,uint16_t iLine)
{
    if(BC_DebugFlag==BC_PAUSE)
    {
        rt_kprintf("������ͣ:%s (file=%s,line=%d)\n",BC_CONTINE_TIPS,iFile,iLine);
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

/*�忨У��*/
static void BC_BoardCal(void)
{
	MBST_Power();
	MBST_Res();
    MBST_ADCCal();
    SBC_ResCal();
    SBC_Res4WCal();
//    SBC_CapCal();
    
    rt_kprintf("\n\n******************�忨У�����*************************\n");
}

/*�忨�Լ�*/
static void BC_BoardSelfTest(void)
{
    SBC_ResSelfTest(); 
    SBC_Res4WSelfTest();
//    SBC_CapSelfTest();
    rt_kprintf("\n\n******************�忨�Լ����*************************\n");
}

/*�忨У���ʼ�������а忨У��ĳ�ʼ�����������ڴ˺�����*/
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

/*�忨У�鿪ʼ
 *uint8_t iType��0���忨У��   1���忨�Լ� 255����ӡ������Ϣ
 */
static void BCStart(uint8_t iType)
{
    if(iType==0xff)
    {
        rt_kprintf("BCStart(0) : ��ʼ�忨У��\n");
        rt_kprintf("BCStart(1) : ��ʼ�忨�Լ�\n");
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

