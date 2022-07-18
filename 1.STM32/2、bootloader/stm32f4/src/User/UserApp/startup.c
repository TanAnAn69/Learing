/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 * 2011-06-05     Bernard      modify for STM32F107 version
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f4xx.h"
#include "board.h"

/**
 * @addtogroup STM32
 */

/*@{*/

extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define STM32_SRAM_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define STM32_SRAM_BEGIN    (&__bss_end)
#endif

extern struct rt_thread *rt_current_thread;
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
    rt_kprintf("\n\r Wrong parameter value detected on\r\n");
    rt_kprintf("       file  %s\r\n", file);
    rt_kprintf("       line  %d\r\n", line);
    rt_kprintf("       thread : %s\r\n", rt_current_thread->name);

    while (1) ;
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
    /* init board */
    rt_hw_board_init();
	/* show version */
    rt_show_version();

    /* init tick */
    rt_system_tick_init();

    /* init kernel object */
    rt_system_object_init();

    /* init timer system */
    rt_system_timer_init();

    rt_system_heap_init((void*)STM32_SRAM_BEGIN, (void*)STM32_SRAM_END);

    /* init scheduler system */
    rt_system_scheduler_init();

    /* init application */
    rt_application_init();

#ifdef RT_USING_FINSH
    /* init finsh */
    finsh_system_init();
    finsh_set_device( FINSH_DEVICE_NAME );
#endif

    /* init timer thread */
    rt_system_timer_thread_init();

    /* init idle thread */
    rt_thread_idle_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    return ;
}

#include "finsh.h"
#define SYSMEM_RESET_VECTOR            0x1fff0000
#define RESET_TO_BOOTLOADER_MAGIC_CODE 0xDEADBEEF
#define BOOT_ARG_ADDRESS               0x2001FFFC
void vCheckBootArg(void)
{
    uint32_t* pArg = (uint32_t*)BOOT_ARG_ADDRESS;
    if (*pArg == RESET_TO_BOOTLOADER_MAGIC_CODE) 
    {
        void (*bootloader)(void) = (void (*)(void)) (*((uint32_t *) (SYSMEM_RESET_VECTOR+4)));
        *pArg = 0;
        RCC_DeInit();
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
        __set_PRIMASK(1);
        __set_MSP(*(uint32_t *)(SYSMEM_RESET_VECTOR));
        bootloader();
    }
}
void EnterBoot()
{
    uint32_t* pArg = (uint32_t*)BOOT_ARG_ADDRESS;
    *pArg = RESET_TO_BOOTLOADER_MAGIC_CODE;
    NVIC_SystemReset();
}
int main(void)
{
    /* disable interrupt first */
    rt_hw_interrupt_disable();
    vCheckBootArg();
    /* startup RT-Thread RTOS */
    rtthread_startup();

    return 0;
}

void Reset(void)
{
    NVIC_SystemReset();
}
FINSH_FUNCTION_EXPORT(EnterBoot,enter bootloader)
FINSH_FUNCTION_EXPORT(Reset,Reset)
/*@}*/
