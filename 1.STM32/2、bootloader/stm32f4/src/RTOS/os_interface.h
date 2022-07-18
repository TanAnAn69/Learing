/*************************************************************************
 * File     : os_interface.h
 * Author	: aaron zhou
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description: 操作系统对外接口头文件
 *
 * Change Logs:
 * Date           Author     Notes
 * 2017-06-21     aaron      first version
 ************************************************************************/
#ifndef __OS_INTERFACE_H
#define __OS_INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>

#include "rtthread.h"
#include "rthw.h"

/*****************************************************/
/*** MACRO DEFINITION AREA ***/
/*****************************************************/
#define Name2Str(name)          (#name)
#define Name(str, number)       (str##_##number)


/*****************************************************/
/*** DATA TYPE AREA ***/
/*****************************************************/
//typedef struct rt_thread        thread_t;  
//typedef rt_thread_t             pThread_t;

//typedef rt_sem_t                pSem_t;

//typedef struct rt_messagequeue  mq_t;
//typedef rt_mq_t                 pMq_t;


/*****************************************************/
/*** VARIABLE DECLARATION AREA ***/
/*****************************************************/

/*****************************************************/
/*** FUNCTION DECLARATION AREA ***/
/*****************************************************/
//void ThreadCreate(const char *name, void (*entry)(void *parameter), void *parameter, uint32_t stack_size,   \
//    uint8_t  priority, uint32_t tick);
//void ThreadCreatePlus(const char *name, void (*entry)(void *parameter), void *parameter, uint32_t stack_size,   \
//    uint8_t  priority, uint32_t tick, pThread_t tid);
//void ThreadDelete(pThread_t tid);

//void ThreadInit(pThread_t tid, const char *name, void (*entry)(void *parameter), void *parameter,void *stack_start, \
//        uint32_t stack_size, uint8_t  priority, uint32_t tick);
//void ThreadDetach(pThread_t tid);

//void SemInit(struct rt_semaphore *sem, const char *strName, uint32_t counter);
//void SemDetach(struct rt_semaphore *sem);
//void rt_sem_take(struct rt_semaphore *sem, uint32_t time);
//void rt_sem_release(struct rt_semaphore *sem);
//void rt_sem_trytake(struct rt_semaphore *sem);


#endif  // __OS_INTERFACE_H


