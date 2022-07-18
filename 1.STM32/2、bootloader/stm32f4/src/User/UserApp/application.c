/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup. 
 */

#include <board.h>
#include <rtthread.h>
#include "stdlib_gpio.h"

#ifdef RT_USING_GDB
#include <gdb_stub.h>
#endif

extern void UserProcess(void);

/*** 网口及FPGA复位信号 ***/ 
void ResetSoft(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOE, GPIO_Pin_0);
    rt_thread_delay(MSec(20));
    GPIO_SetBits(GPIOE, GPIO_Pin_0);
    rt_thread_delay(MSec(20));
}


void rt_init_thread_entry(void* parameter)
{
    ResetSoft();
    /* GDB STUB */
#ifdef RT_USING_GDB
    gdb_set_device("uart6");
    gdb_start();
#endif

    /*** 主进程 ***/
    UserProcess();
}

extern void BoardInit(void);

static void LEDPortInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
}
struct rt_timer LEDTimer;//用于主板LED等闪烁的定时器
void LEDTimerCallBack(void *parameter)
{
    GPIOA->ODR ^= GPIO_Pin_11;
}

int rt_application_init()
{
    rt_thread_t tid;
    
    LEDPortInit();
    rt_timer_init(&LEDTimer,"LEDTimer",LEDTimerCallBack,RT_NULL,25,RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(&LEDTimer);
    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    
    return 0;
}

/*@}*/


