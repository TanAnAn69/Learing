/*************************************************************************
 * File     : os_interface.c
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
#include "os_interface.h"

/*****************************************************/
/*** MACRO DEFINITION AREA ***/
/*****************************************************/

/*****************************************************/
/*** GLOBAL VARIABLE AREA ***/
/*****************************************************/

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
void SemInit(struct rt_semaphore *sem, const char *strName, uint32_t counter)
{
    rt_err_t result;
	result = rt_sem_init(sem,strName,counter,RT_IPC_FLAG_FIFO);
	if(result != RT_EOK)
	{
		rt_kprintf("%s init fail.\n", strName);                                 //TODO:将警告信息的打印封装出去
	}
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
void SemDetach(struct rt_semaphore *sem)
{
    rt_err_t result;
    result = rt_sem_detach(sem);
    if(result != RT_EOK)
    {
        rt_kprintf("%s detach fail.\n", Name2Str(sem));
    }
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
//void SemTake(struct rt_semaphore *sem, uint32_t time)
//{
//    rt_sem_take(sem, time);
//}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
//void SemRelease(struct rt_semaphore *sem)
//{
//    rt_sem_release(sem);
//}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
//void SemTryTake(struct rt_semaphore *sem)
//{
//    rt_sem_trytake(sem);
//}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
void ThreadCreate(const char *name, void (*entry)(void *parameter), void *parameter, uint32_t stack_size,   \
    uint8_t  priority, uint32_t tick)
{
    pThread_t tid;
    tid = rt_thread_create(name, entry, parameter, stack_size,  priority, tick);
    if(tid != RT_NULL)
        rt_thread_startup(tid);
    else
        rt_kprintf("%s thread create fail.\n", name);
}

void ThreadCreatePlus(const char *name, void (*entry)(void *parameter), void *parameter, uint32_t stack_size,   \
    uint8_t  priority, uint32_t tick, pThread_t tid)
{
    tid = rt_thread_create(name, entry, parameter, stack_size,  priority, tick);
    if(tid != RT_NULL)
        rt_thread_startup(tid);
    else
        rt_kprintf("%s thread create fail.\n", name);
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
void ThreadDelete(pThread_t tid)
{
    rt_err_t result;
    result = rt_thread_delete(tid);
    if(result != RT_EOK)
        rt_kprintf("%s thread delete fail.\n", tid->name);
    tid = NULL;
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
void ThreadInit(pThread_t tid, const char *name, void (*entry)(void *parameter), void *parameter,void *stack_start, 
        uint32_t stack_size, uint8_t  priority, uint32_t tick)
{
    rt_err_t result;
    result = rt_thread_init(tid,name,entry, RT_NULL,stack_start, stack_size, priority, tick);
	if (result == RT_EOK)
	{
		rt_thread_startup(tid);
		rt_kprintf("%s thread start \n", name);
	}
}

/**
  * @brief  .
  * @note   .
  * @param  .
  * @retval .
  **/
void ThreadDetach(pThread_t tid)
{
    rt_err_t result;
    result = rt_thread_detach(tid);
    if (result == RT_EOK)
	{
		rt_kprintf("%s thread detach \n", tid->name);
	}
}
  
 
