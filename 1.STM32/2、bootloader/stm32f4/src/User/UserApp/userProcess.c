/*************************************************************************
 * File     : userProcess.c
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
#include "bsp_88e6060.h"
#include "bsp_eeprom.h"
#include "networkprocess.h"
#include "bsp_common.h"
#include "limitcfg.h"
#include "networkcfg.h"


extern void DBG_Init(uint16_t iAddr,uint16_t iSize);
extern void NetworkInit(void);
extern int eth_system_device_init(void);
extern const sEepromBlockInfo EepromBlockInfo[];//eeprom地址块分配信息
extern uint16_t EEPBNum;
extern uint16_t LIMITNum;
extern uint16_t SCFGNum;
extern void LimitCfgInit(void);
extern void NET_CfgInit(uint16_t iEepromAddr,uint16_t iEepromSize);
extern void NET_LocalDefault(uint8_t iIP);
extern void EnterUser(void);
/*****************************************************/
/*** MACRO DEFINITION AREA ***/
/*****************************************************/

/*****************************************************/
/*** GLOBAL VARIABLE AREA ***/
/*****************************************************/
ALIGN(RT_ALIGN_SIZE)

#define RT_NETWORK_COMMU_THREAD_PRIO		17
struct rt_thread network_commu_thread;
rt_uint8_t network_commu_stack[1024];
extern void network_commu_thread_entry(void* parameter);

#define RT_NETWORK_PROCESS_THREAD_PRIO		19
struct rt_thread network_process_thread;
rt_uint8_t network_process_stack[1024];
extern void network_process_thread_entry(void* parameter);


#define RT_UPDATE_THREAD_PRIO		22
struct rt_thread update_thread;
rt_uint8_t update_stack[1024];
extern void update_thread_entry(void* parameter);

/*返回1说明boot按键按下，返回0说明boot按键没有被按下*/
uint8_t ReadBootKey(char *iPin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_TypeDef *port;
    uint16_t pin_x;   //0~15;
    
    if(rt_strlen(iPin)!=3 && rt_strlen(iPin)!=4)
    {
        return 0;
    }
    if(iPin[0]!='P')
    {
        return 0;
    }
    if(iPin[1]<'A'||iPin[1]>'G')
    {
        return 0;
    }    
    if(atoi(&iPin[2])>15)
    {
        return 0;
    }

    port=(GPIO_TypeDef *)(GPIOA_BASE + (iPin[1]-'A')*(GPIOB_BASE-GPIOA_BASE));
    pin_x=(1<<(atoi(&iPin[2])));
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

    RCC_AHB1PeriphClockCmd((1<<(iPin[1]-'A')), ENABLE);

    GPIO_InitStructure.GPIO_Pin = pin_x;
    GPIO_Init(port, &GPIO_InitStructure);
    rt_thread_delay(MSec(10));
    return GPIO_ReadInputDataBit(port,pin_x);   
}

void UserProcess(void)
{
    rt_err_t result;
    uint8_t bootkey;
    
    EEPROM_Init();
    
    SCFG_Init(EepromBlockInfo,EEPBNum);   

    NetworkInit();//网络初始化必须放在机台配置初始化之后即函数LimitCfgInit()之后 ///todo:待编写:未初始化就使用，则报错
    bootkey = ReadBootKey(NetLocalAddr.BootKey);
    rt_kprintf("BootKey=%d : 1->Enter boot.  0->Enter User\n",bootkey );
    rt_kprintf("BootFlag=%d : 1->Enter boot.  0->Enter User\n" ,NetLocalAddr.BootFlag);
    if((NetLocalAddr.BootFlag == 0) && (bootkey==0))
    {
        EnterUser();
    }
    rt_kprintf("enter boot\n");
    //Network communication
    result = rt_thread_init(&network_commu_thread, "nwkc", network_commu_thread_entry, RT_NULL,
	(rt_uint8_t*)&network_commu_stack[0], sizeof(network_commu_stack), RT_NETWORK_COMMU_THREAD_PRIO, 20);
	if (result == RT_EOK)
    {
		rt_thread_startup(&network_commu_thread);
	}
    
    //Network process
    result = rt_thread_init(&network_process_thread, "nwkp", network_process_thread_entry, RT_NULL,
	(rt_uint8_t*)&network_process_stack[0], sizeof(network_process_stack), RT_NETWORK_PROCESS_THREAD_PRIO, 20);
	if (result == RT_EOK)
    {
		rt_thread_startup(&network_process_thread);
	}
    
    //firmware updata thread
    result = rt_thread_init(&update_thread, "updt", update_thread_entry, RT_NULL,
	(rt_uint8_t*)&update_stack[0], sizeof(update_stack), RT_UPDATE_THREAD_PRIO, 20);
	if (result == RT_EOK)
    {
		rt_thread_startup(&update_thread);
	}
}

