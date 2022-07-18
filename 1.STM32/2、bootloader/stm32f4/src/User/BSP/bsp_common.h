#ifndef __BSP_COMMON_H
#define __BSP_COMMON_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "rtthread.h"
#include "rthw.h"
#include "rtdef.h"
#include "stdint.h"
#include <core_cm4.h>

/*指针检查*/
//stm32f4可用的所有指针
#define POINT_CHECK(p) ((p>=0x8000000&&p<=0x8100000) || (p>=0x20000000&&p<=0x20020000) || (p>=0x10000000&&p<=0x10010000))
//指向flash的指针
#define POINT_FLASH_CHECK(p) (p>=0x8000000&&p<=0x8100000)
//指向RAAM的指针
#define POINT_RAM_CHECK(p) ((p>=0x20000000&&p<=0x20020000) || (p>=0x10000000&&p<=0x10010000))

/*以下宏定义用于陷阱处理*/
#define TRAP_VALUE8 0xAA
#define TRAP_CHECK8(iAddr,iTips) {if( *((uint8_t *)(iAddr))  != TRAP_VALUE8 ) {rt_kprintf("\n%s,%d: %s :Illegal Access RAM 0x%08x, Value=%x\n",__FUNCTION__,__LINE__,iTips,iAddr,*((uint8_t *)iAddr));while(1);}}
#define TRAP_VALUE32 0xAAAAAAAA
#define TRAP_SET32(iAddr)  (*((uint32_t *)(iAddr)) = TRAP_VALUE32)
#define TRAP_CHECK32(iAddr,iTips) {if( *((uint32_t *)(iAddr))  != TRAP_VALUE32) {rt_kprintf("\n%s,%d: %s :Illegal Access RAM 0x%08x, Value=%x\n",__FUNCTION__,__LINE__,iTips,iAddr,*((uint32_t *)iAddr));while(1);}}
#define FLOATP(p)   ((float *)(&(((float *)(p))[1])))  //将做了陷阱处理的float类型的内存指针转换为float类型的有效数据指针，只适用于一维数组

#define ATOMIC_VAR   rt_base_t level;
#define ATOMIC(exp) {level = rt_hw_interrupt_disable();exp;rt_hw_interrupt_enable(level);}

/*下述宏定义用于测量us级别的代码运行时间*/ 
#define TIME_MEASURE_VAR_DEF   rt_uint32_t systickstart,systickend; //变量定义  
#define TIME_MEASURE_START     systickstart=SysTick->VAL; //时间测量开始
//时间测量结束,并将时间打印出来
#define TIME_MEASURE_STOP      {systickend=SysTick->VAL; \
                                rt_kprintf("Run time=%.3f us , start=0x%08x , end=0x%08x , diff=0x%08x , load=0x%08x\n", \
                                           1.0f*(systickstart-systickend)/((SysTick->LOAD+1)/(1000000.0f/RT_TICK_PER_SECOND)), \
                                           systickstart,systickend,systickstart-systickend,SysTick->LOAD);}

#define PRINT_INIT_START() rt_kprintf("\n[%s] Init Start ...\n",__FUNCTION__);
#define PRINT_INIT_OK()   rt_kprintf("[%s] Init OK \n",__FUNCTION__);
#define PRINT_INIT_FAIL()   rt_kprintf("[%s] Init Fail \n",__FUNCTION__);
#define PRINT_FUNC_ENTER() rt_kprintf("Enter func :%s \n",__FUNCTION__);
#define PRINT_FUNC_EXIT(); rt_kprintf("Exit func :%s \n",__FUNCTION__);

#define FWERR_OK 0 //firmware error ok : 无错误
#define FINSH_TYPE static void //用于定义函数类型，凡是用此类型定义的函数都是用于finsh组件的
/******************************************************************************************************************
 *TIME_START,TIME_END：打印函数或代码段的运行的时间
 *使用方法：在函数变量定义区先定义一个记录时间的变量，例如uint32_t tick;
 *          然后在函数开始处调用TIME_START(tick);在函数结尾处调用TIME_END(tick),即可在串口终端打印出此函数运行时间.
 *          此宏定义可以放在一个函数内的多个地方，以测试不同代码段的运行时间
 *******************************************************************************************************************/
#define TIME_START() {tick = rt_tick_get();} 
#define TIME_END()   {rt_kprintf("[Run Time] Func : %s , Line : %d = %d ms ! \n",__FUNCTION__,__LINE__,(rt_tick_get() - tick)*10);}

/*FW_LOG：打印调用此宏的文件名，函数名称，以及此宏所在行号（默认有换行符，如需要格式化输出，可不加换行符）
  使用范围：如果程序出现死机或其他异常需要定位问题出现在哪里，可以在可疑地点调用此函数，以快速定位问题*/ 
#define FW_LOG(format,...) (rt_kprintf("Func : %s , Line : %d "format"\n",__FUNCTION__,__LINE__,##__VA_ARGS__)) 

/*错误诊断*/
#define FWAssertError(a,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nerror: \n\t[Desc]\t"format"",##__VA_ARGS__); \
        rt_kprintf("\t[Func]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",__FUNCTION__,__LINE__,rt_current_thread->name);while(1) rt_thread_delay(100);}}
#define FWAssertWarning(a,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nwarning: \n\t[Desc]\t"format"",##__VA_ARGS__);\
        rt_kprintf("\t[Func]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",__FUNCTION__,__LINE__,rt_current_thread->name);}} 
#define FWAssertErrorEx(a,pFile,pFunc,bLine,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nerror: \n\t[Desc]\t"format"",##__VA_ARGS__); \
        rt_kprintf("\t[Func]\t%s\n\t[file]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",pFunc,pFile,bLine,rt_current_thread->name);while(1) rt_thread_delay(100);}}
#define FWAssertWarningEx(a,pFile,pFunc,bLine,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nwarning: \n\t[Desc]\t"format"",##__VA_ARGS__);\
        rt_kprintf("\t[Func]\t%s\n\t[file]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",pFunc,pFile,bLine,rt_current_thread->name);}} 

/*数组溢出检查，ArrayCheck3,4分别代表需要输入3,4个参数，默认是数组最后四个字节用于溢出检查，因此定义数组时，需要多出4个字节*/
#define PRINT_ARRAY_FREE_SPACE
#define ARRAY_WARNING_SPACE 4 //最少剩余多少空间（以字节为单位，最后几个字节会被填充为固定的字符作为数组结束标志，如果数组结束标志不存在了，则认为数组溢出） 
#define ARRAY_INIT_VALUE '~'	//默认的用于数组初始化的数据(四个符号‘~'对应的浮点数基本上等于无穷大，并且在C语言中基本不会出现连续几个符号‘~’)
#define ArrayCheck(pArray,bLen)  ArrayOverFlowCheck(pArray,bLen,ARRAY_DEFAULT_VALUE,ARRAY_WARNING_SPACE,(char *)__FILE__,(char *)__FUNCTION__,__LINE__)
#define ArrayCheck3(pArray,bLen,bInitChar)  ArrayOverFlowCheck(pArray,bLen,bInitChar,ARRAY_WARNING_SPACE,(char *)__FILE__,(char *)__FUNCTION__,__LINE__)
#define ArrayCheck4(pArray,bLen,bInitChar,bWarningSpace)  ArrayOverFlowCheck(pArray,bLen,bInitChar,bWarningSpace,(char *)__FILE__,(char *)__FUNCTION__,__LINE__)
        

/*调试选项*/
struct DebugOptDef
{
    uint8_t *Opt;
    const char *Name; 
    const char *Desc; 
};

/*var:调试变量的名字
  name：调试选项名字，此名字用于存储到eeprom中，需要尽可能短，建议6个字节以内
  decs：调试选项的详细描述*/
#define DBG_EXPORT(var,name,decs) \
const char debugopt##name##_name[12] SECTION(".rodata.name") = #name; \
const char debugopt_##name##_decs[] SECTION(".rodata.name") = #decs; \
const struct DebugOptDef debugopt_##name SECTION("DebugOpt")= \
{                            \
   &var,                     \
   debugopt##name##_name,  \
   debugopt_##name##_decs,  \
};


/*文件版本*/
typedef struct 
{
    const char *Name; 
    const char *version; 
}sFileVersion;
/*var:文件版本
  name：文件名称
  decs：调试选项的详细描述*/
#define FILE_VERSION_EXPORT(name,version) \
const char file_version_##name##_name[] SECTION(".rodata.name") = #name; \
const char file_version_##name##_ver[] SECTION(".rodata.name") = #version; \
const sFileVersion  file_version_##name##_info SECTION("FileVersion")= \
{                            \
   file_version_##name##_name,  \
   file_version_##name##_ver,  \
};

extern rt_thread_t rt_current_thread;
extern float GetAverage(float *p, uint8_t maxnum, uint8_t filternum_min,uint8_t filternum_max);
extern void ArrayInit(void *pArray,char iValue,uint16_t iLen);
extern struct rt_thread *rt_current_thread;
extern void DBG_Save(const char *iName,uint8_t iDebugOpt);

#endif

