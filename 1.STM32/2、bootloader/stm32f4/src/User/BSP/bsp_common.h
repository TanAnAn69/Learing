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

/*ָ����*/
//stm32f4���õ�����ָ��
#define POINT_CHECK(p) ((p>=0x8000000&&p<=0x8100000) || (p>=0x20000000&&p<=0x20020000) || (p>=0x10000000&&p<=0x10010000))
//ָ��flash��ָ��
#define POINT_FLASH_CHECK(p) (p>=0x8000000&&p<=0x8100000)
//ָ��RAAM��ָ��
#define POINT_RAM_CHECK(p) ((p>=0x20000000&&p<=0x20020000) || (p>=0x10000000&&p<=0x10010000))

/*���º궨���������崦��*/
#define TRAP_VALUE8 0xAA
#define TRAP_CHECK8(iAddr,iTips) {if( *((uint8_t *)(iAddr))  != TRAP_VALUE8 ) {rt_kprintf("\n%s,%d: %s :Illegal Access RAM 0x%08x, Value=%x\n",__FUNCTION__,__LINE__,iTips,iAddr,*((uint8_t *)iAddr));while(1);}}
#define TRAP_VALUE32 0xAAAAAAAA
#define TRAP_SET32(iAddr)  (*((uint32_t *)(iAddr)) = TRAP_VALUE32)
#define TRAP_CHECK32(iAddr,iTips) {if( *((uint32_t *)(iAddr))  != TRAP_VALUE32) {rt_kprintf("\n%s,%d: %s :Illegal Access RAM 0x%08x, Value=%x\n",__FUNCTION__,__LINE__,iTips,iAddr,*((uint32_t *)iAddr));while(1);}}
#define FLOATP(p)   ((float *)(&(((float *)(p))[1])))  //���������崦���float���͵��ڴ�ָ��ת��Ϊfloat���͵���Ч����ָ�룬ֻ������һά����

#define ATOMIC_VAR   rt_base_t level;
#define ATOMIC(exp) {level = rt_hw_interrupt_disable();exp;rt_hw_interrupt_enable(level);}

/*�����궨�����ڲ���us����Ĵ�������ʱ��*/ 
#define TIME_MEASURE_VAR_DEF   rt_uint32_t systickstart,systickend; //��������  
#define TIME_MEASURE_START     systickstart=SysTick->VAL; //ʱ�������ʼ
//ʱ���������,����ʱ���ӡ����
#define TIME_MEASURE_STOP      {systickend=SysTick->VAL; \
                                rt_kprintf("Run time=%.3f us , start=0x%08x , end=0x%08x , diff=0x%08x , load=0x%08x\n", \
                                           1.0f*(systickstart-systickend)/((SysTick->LOAD+1)/(1000000.0f/RT_TICK_PER_SECOND)), \
                                           systickstart,systickend,systickstart-systickend,SysTick->LOAD);}

#define PRINT_INIT_START() rt_kprintf("\n[%s] Init Start ...\n",__FUNCTION__);
#define PRINT_INIT_OK()   rt_kprintf("[%s] Init OK \n",__FUNCTION__);
#define PRINT_INIT_FAIL()   rt_kprintf("[%s] Init Fail \n",__FUNCTION__);
#define PRINT_FUNC_ENTER() rt_kprintf("Enter func :%s \n",__FUNCTION__);
#define PRINT_FUNC_EXIT(); rt_kprintf("Exit func :%s \n",__FUNCTION__);

#define FWERR_OK 0 //firmware error ok : �޴���
#define FINSH_TYPE static void //���ڶ��庯�����ͣ������ô����Ͷ���ĺ�����������finsh�����
/******************************************************************************************************************
 *TIME_START,TIME_END����ӡ���������ε����е�ʱ��
 *ʹ�÷������ں��������������ȶ���һ����¼ʱ��ı���������uint32_t tick;
 *          Ȼ���ں�����ʼ������TIME_START(tick);�ں�����β������TIME_END(tick),�����ڴ����ն˴�ӡ���˺�������ʱ��.
 *          �˺궨����Է���һ�������ڵĶ���ط����Բ��Բ�ͬ����ε�����ʱ��
 *******************************************************************************************************************/
#define TIME_START() {tick = rt_tick_get();} 
#define TIME_END()   {rt_kprintf("[Run Time] Func : %s , Line : %d = %d ms ! \n",__FUNCTION__,__LINE__,(rt_tick_get() - tick)*10);}

/*FW_LOG����ӡ���ô˺���ļ������������ƣ��Լ��˺������кţ�Ĭ���л��з�������Ҫ��ʽ��������ɲ��ӻ��з���
  ʹ�÷�Χ�����������������������쳣��Ҫ��λ�����������������ڿ��ɵص���ô˺������Կ��ٶ�λ����*/ 
#define FW_LOG(format,...) (rt_kprintf("Func : %s , Line : %d "format"\n",__FUNCTION__,__LINE__,##__VA_ARGS__)) 

/*�������*/
#define FWAssertError(a,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nerror: \n\t[Desc]\t"format"",##__VA_ARGS__); \
        rt_kprintf("\t[Func]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",__FUNCTION__,__LINE__,rt_current_thread->name);while(1) rt_thread_delay(100);}}
#define FWAssertWarning(a,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nwarning: \n\t[Desc]\t"format"",##__VA_ARGS__);\
        rt_kprintf("\t[Func]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",__FUNCTION__,__LINE__,rt_current_thread->name);}} 
#define FWAssertErrorEx(a,pFile,pFunc,bLine,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nerror: \n\t[Desc]\t"format"",##__VA_ARGS__); \
        rt_kprintf("\t[Func]\t%s\n\t[file]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",pFunc,pFile,bLine,rt_current_thread->name);while(1) rt_thread_delay(100);}}
#define FWAssertWarningEx(a,pFile,pFunc,bLine,format,...) {uint8_t i$Flag=a; if(!(i$Flag)){rt_kprintf("\n\nwarning: \n\t[Desc]\t"format"",##__VA_ARGS__);\
        rt_kprintf("\t[Func]\t%s\n\t[file]\t%s\n\t[line]\t%d\n\t[thread]\t%s\n ",pFunc,pFile,bLine,rt_current_thread->name);}} 

/*���������飬ArrayCheck3,4�ֱ������Ҫ����3,4��������Ĭ������������ĸ��ֽ����������飬��˶�������ʱ����Ҫ���4���ֽ�*/
#define PRINT_ARRAY_FREE_SPACE
#define ARRAY_WARNING_SPACE 4 //����ʣ����ٿռ䣨���ֽ�Ϊ��λ����󼸸��ֽڻᱻ���Ϊ�̶����ַ���Ϊ���������־��������������־�������ˣ�����Ϊ��������� 
#define ARRAY_INIT_VALUE '~'	//Ĭ�ϵ����������ʼ��������(�ĸ����š�~'��Ӧ�ĸ����������ϵ�������󣬲�����C�����л���������������������š�~��)
#define ArrayCheck(pArray,bLen)  ArrayOverFlowCheck(pArray,bLen,ARRAY_DEFAULT_VALUE,ARRAY_WARNING_SPACE,(char *)__FILE__,(char *)__FUNCTION__,__LINE__)
#define ArrayCheck3(pArray,bLen,bInitChar)  ArrayOverFlowCheck(pArray,bLen,bInitChar,ARRAY_WARNING_SPACE,(char *)__FILE__,(char *)__FUNCTION__,__LINE__)
#define ArrayCheck4(pArray,bLen,bInitChar,bWarningSpace)  ArrayOverFlowCheck(pArray,bLen,bInitChar,bWarningSpace,(char *)__FILE__,(char *)__FUNCTION__,__LINE__)
        

/*����ѡ��*/
struct DebugOptDef
{
    uint8_t *Opt;
    const char *Name; 
    const char *Desc; 
};

/*var:���Ա���������
  name������ѡ�����֣����������ڴ洢��eeprom�У���Ҫ�����̣ܶ�����6���ֽ�����
  decs������ѡ�����ϸ����*/
#define DBG_EXPORT(var,name,decs) \
const char debugopt##name##_name[12] SECTION(".rodata.name") = #name; \
const char debugopt_##name##_decs[] SECTION(".rodata.name") = #decs; \
const struct DebugOptDef debugopt_##name SECTION("DebugOpt")= \
{                            \
   &var,                     \
   debugopt##name##_name,  \
   debugopt_##name##_decs,  \
};


/*�ļ��汾*/
typedef struct 
{
    const char *Name; 
    const char *version; 
}sFileVersion;
/*var:�ļ��汾
  name���ļ�����
  decs������ѡ�����ϸ����*/
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

