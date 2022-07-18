#include "stdtest_common.h"
#include "stdlib_gpio.h"

#define IO_GROUP_NUM 22
typedef struct
{
    const char *OutName;
    uint16_t OutAddr;
    const char *InName;
    uint16_t InAddr;
}sIOInfo;

const sIOInfo IOInfo[IO_GROUP_NUM]=
{
    {"OUT1",0x214,"IN1",0x1010},
    {"OUT1",0x214,"IN2",0x1011},
    {"OUT2",0x215,"IN3",0x1012},
    {"OUT2",0x215,"IN4",0x1013},
    {"OUT3",0x216,"IN5",0x1014},
    {"OUT3",0x216,"IN6",0x1015}, 
    {"OUT4",0x217,"IN7",0x1016},
    {"OUT4",0x217,"IN8",0x1017},
    {"OUT5",0x310,"IN9",0x1110},
    {"OUT5",0x310,"IN10",0x1111},
    {"OUT6",0x311,"IN11",0x1112},
    {"OUT6",0x311,"IN12",0x1113},
    {"OUT7",0x312,"IN13",0x1114},
    {"OUT7",0x312,"IN14",0x1115},
    {"OUT8",0x313,"IN15",0x1116},
    {"OUT8",0x313,"IN16",0x1117},
    {"OUT9",0x314,"IN17",0x1210},
    {"OUT9",0x314,"IN18",0x1211},
    {"OUT10",0x315,"IN19",0x1212},
    {"OUT10",0x315,"IN20",0x1213},
    {"OUT11",0x316,"IN19",0x1212},
    {"OUT12",0x317,"IN20",0x1213},
};

void IOBoardSelfTest(void)
{
    uint8_t i=0,data=0,result=1;
    for(i=0;i<IO_GROUP_NUM;i++)
    {
        SubSetIO(IOInfo[i].OutAddr,1);
        _delay_ms(1);
        data=SubReadIO(IOInfo[i].OutAddr);
        rt_kprintf("%s=1,%s=%d\n",IOInfo[i].OutName,IOInfo[i].InName,data);
        if(data != 1) 
        {
            result=0;
            rt_kprintf("Error : In value not equal Out value !\n\n\n");
        }
        
        SubSetIO(IOInfo[i].OutAddr,0);
        _delay_ms(1);
        data=SubReadIO(IOInfo[i].OutAddr);
        rt_kprintf("%s=0,%s=%d\n",IOInfo[i].OutName,IOInfo[i].InName,data);
        if(data != 0) 
        {
            result=0;
            rt_kprintf("Error : In value not equal Out value !\n\n\n");
        }
    }
    if(result==1) rt_kprintf("IO Board Selftest Pass !\n");
    else rt_kprintf("IO Board Selftest Fail !\n");
}

FINSH_FUNCTION_EXPORT(IOBoardSelfTest,IOBoardSelfTest())
