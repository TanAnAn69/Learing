/*
 * 缩略词含义：
 * SBC_:switch board calibration （swithc板校准）
 * SBST_:switch board selftest （swithc板自检）
 */
#include "stdtest_common.h"
extern uint16_t BC_EepromAddr;

#define EPA_B_RES_CAL (BC_EepromAddr+0)


/*电阻1和电阻2的校准电压*/
typedef struct 
{
	float V1; 
	float V2;
}sResCalVolt;

/*电阻校准系数*/
typedef struct
{ 
	uint8_t initflag;
    float k; 
	float b;
}sResCalFactor;

/*电阻校验时需要用到的相关信息：全部以Ohm为单位*/
typedef struct
{
	const char *RangeName;//档位名称
    uint16_t Range; //需要校准的档位

    const char *R1Name; //电阻1名称
	uint16_t R1HPin; //电阻1的高引脚
	uint16_t R1LPin; //电阻1的低引脚
    float Res1;     //电阻1的阻值
    
    const char *R2Name; //电阻2名称    
	uint16_t R2HPin; //电阻2的高引脚
	uint16_t R2LPin; //电阻2的低引脚
    float Res2;      //电阻2的阻值

    float V1Min;//电阻1两端电压最小值
    float V1Max;//电阻1两端电压最大值
    float V2Min;//电阻2两端电压最小值
    float V2Max;//电阻2两端电压最大 值
	uint16_t flag;    //是否需要校验的标志位
}sResCalInfo;

/*两线电阻自检时需要用到的相关信息：全部以Ohm为单位*/
typedef struct
{
    const char *Name; //电阻名称
    uint16_t Range; //测试此电阻的档位
	uint16_t HPin; //电阻的高引脚
	uint16_t LPin; //电阻的低引脚
    float Res;     //电阻的阻值
	uint16_t flag; //是否需要自检
}sResSelfTestInfo;

/*用于两线校准时的电阻信息*/
static uint8_t ResCalFlag=0; //res校验系数是否正确  0:Res校验系数错误  1:res校验系数正确
static uint8_t ResCalInitFlag=0; //res校验系数是否初始化，0：系数未初始化  1：系数已经初始化
static sResCalVolt ResCalVolt[RES_RANGE_NUM];
static sResCalFactor ResCalFactor[RES_RANGE_NUM];
static const sResCalInfo ResCalInfo[RES_RANGE_NUM]=
{
    /*RangeName      Range         R1Name,R1HPin,R1LPin,Res1  R2Name,R2HPin,R2LPin,Res2 ; V1Min,V1Max,V2Min,V2Max,flag*/
    {"RES_RANGE_10R",RES_RANGE_10R,"R12",1,2,2.21,    "R11",5,6,4.99    ,0.393,0.729,0.431,0.801,BC_CAL},
    {"RES_RANGE_100R",RES_RANGE_100R,"R2",9,10,33,    "R3",13,14,75    ,0.472,0.708,0.808,1.212,BC_CAL},
    {"RES_RANGE_1KR",RES_RANGE_1KR,"R1",17,18,330,    "R4",21,22,750    ,0.637,0.779,1.389,1.698,BC_CAL},
    {"RES_RANGE_10KR",RES_RANGE_10KR,"R6",25,26,3300,    "R5",29,30,6200    ,0.595,0.728,1.115,1.362,BC_CAL},
    {"RES_RANGE_100KR",RES_RANGE_100KR,"R7",33,34,30000,    "R8",37,38,73200    ,0.754,0.922,1.2,1.467,BC_CAL},
    {"RES_RANGE_1MR",RES_RANGE_1MR,"R10",41,42,191000,    "R9",45,46,470000    ,0.564,0.689,0.988,1.207,BC_CAL},
    {"RES_RANGE_10MR",RES_RANGE_10MR,"null",0,0,0,    "null",0,0,0    ,0,0,0,0,BC_NCAL},
};

/*用于两线自检的电阻信息*/
#define RES_SELF_TEST_NUM 12
static const sResSelfTestInfo ResSelfTestInfo[RES_SELF_TEST_NUM]=
{    
    /*Name,Range,HPin,LPin,unit,flag*/
    {"R12", RES_RANGE_10R,    1, 2, 3.33,  BC_SELF_TEST},
    {"R11", RES_RANGE_10R,    5, 6, 8.3,   BC_SELF_TEST},
    {"R2",  RES_RANGE_100R,  9, 10,50.9,  BC_SELF_TEST},
    {"R3",  RES_RANGE_100R,  13,14,100,   BC_SELF_TEST},
    {"R1",  RES_RANGE_1KR,  17,18,148.9, BC_SELF_TEST},
    {"R4",  RES_RANGE_1KR,  21,22,510.3, BC_SELF_TEST},
    {"R6",  RES_RANGE_10KR,  25,26,1.498, BC_SELF_TEST},
    {"R5",  RES_RANGE_10KR,  29,30,9.12,  BC_SELF_TEST},
    {"R7",  RES_RANGE_100KR,33,34,20,    BC_SELF_TEST},
    {"R8",  RES_RANGE_100KR,37,38,55.96, BC_SELF_TEST},
    {"R10", RES_RANGE_1MR, 41,42,100.59,BC_SELF_TEST},
    {"R9",  RES_RANGE_1MR, 45,46,570.18,BC_SELF_TEST},
};

/*获取取校验系数*/
void SBC_GetResCalFactor(float *k,float *b)
{
    float res1,res2,v1,v2;
    FWAssertError(ResCalInitFlag,"Res 校验系数未初始化!\n");
    FWAssertError(ResCurRange<RES_RANGE_NUM,"Range error!\n");
    FWAssertError(ResCalFlag,"Res 校验系数存在错误，请重新校验!\n");
    FWAssertError(ResCalInfo[ResCurRange].flag==BC_CAL,"请不要使用未校验的档位 = %s\n",ResCalInfo[ResCurRange].RangeName);
    
    if(ResCalFactor[ResCurRange].initflag==0)
    {
        res1=ResCalInfo[ResCurRange].Res1;
        res2=ResCalInfo[ResCurRange].Res2;
        v1=ResCalVolt[ResCurRange].V1;
        v2=ResCalVolt[ResCurRange].V2;
        
        /*计算系数*/
        if(ResTestMethod==RES_CC_TEST)
        {
            ResCalFactor[ResCurRange].k=(res1-res2)/(v1-v2); //k=(y1-y2)/(x1-x2)
            ResCalFactor[ResCurRange].b=res1-ResCalFactor[ResCurRange].k*v1;//b=y1-kx1
        }
        else if(ResTestMethod==RES_CV_TEST)
        {
            ResCalFactor[ResCurRange].b=res1*res2*(v2-v1)/(v1*res2-v2*res1);
            ResCalFactor[ResCurRange].k=v1/res1*(ResCalFactor[ResCurRange].b+res1);
        }
        else
        {
            FWAssertError(0,"ResTestMethod=%d !\n",ResTestMethod);
        }
        ResCalFactor[ResCurRange].initflag=1;
    }
    
    *k=ResCalFactor[ResCurRange].k;
    *b=ResCalFactor[ResCurRange].b;
}

/*Res校验系数初始化*/
void SBC_ResCalInit(void)
{
    uint8_t i;
    float v1,v2;
    
    PRINT_INIT_START();
    ResCalFlag=1;
    for(i=0;i<RES_RANGE_NUM;i++)
    {
        if(ResCalInfo[i].flag==BC_NCAL) continue;
        
        /*从eeprom中读取电阻两端的校验电压*/
        v1=EEPROM_Rdf(EPA_B_RES_CAL+i*8);
        v2=EEPROM_Rdf(EPA_B_RES_CAL+4+i*8);
        
        /*打印系数信息*/
        rt_kprintf("res cal:%s , V1=%f[%f,%f] add=%d , V2=%f[%f,%f] add=%d\n",ResCalInfo[i].RangeName,
                    v1,ResCalInfo[i].V1Min ,ResCalInfo[i].V1Max,EPA_B_RES_CAL+i*8,   v2,ResCalInfo[i].V2Min,ResCalInfo[i].V2Max,EPA_B_RES_CAL+4+i*8);
        /*判断电压是否在门限内*/
        if(v1>=ResCalInfo[i].V1Min && v1<=ResCalInfo[i].V1Max && v2>=ResCalInfo[i].V2Min && v2<=ResCalInfo[i].V2Max)
        {
            ResCalVolt[i].V1=v1;
            ResCalVolt[i].V2=v2;
        }
        else
        {
            ResCalFlag=0;//校验系数错误
            rt_kprintf("--------档位%s的校验系数错误----------\n\n",ResCalInfo[i].RangeName);
        }
    }
    ResCalInitFlag=1;
    PRINT_INIT_OK();
}

/*设置为默认的校验系数：默认的校验系数为门限的中间值
 注：此功能只有在需要测试电阻而手边又没有校验好的板子时使用*/
static void SetResCalDefault(void)
{
    uint8_t i=0;
    for(i=0;i<RES_RANGE_NUM;i++)
    {
        ResCalVolt[i].V1=(ResCalInfo[i].V1Min+ResCalInfo[i].V1Max)/2;
        ResCalVolt[i].V2=(ResCalInfo[i].V2Min+ResCalInfo[i].V2Max)/2;
        EEPROM_Wrf(EPA_B_RES_CAL+i*8,ResCalVolt[i].V1);
        EEPROM_Wrf(EPA_B_RES_CAL+4+i*8,ResCalVolt[i].V2);
        /*打印系数信息*/
        rt_kprintf("res cal:%s , V1=%f[%f,%f] add=%d , V2=%f[%f,%f] add=%d\n",
                    ResCalInfo[i].RangeName, ResCalVolt[i].V1,ResCalInfo[i].V1Min,ResCalInfo[i].V1Max,EPA_B_RES_CAL+i*8,
                    ResCalVolt[i].V2,ResCalInfo[i].V2Min,ResCalInfo[i].V2Max,EPA_B_RES_CAL+4+i*8);
    }
    ResCalFlag=1;
    rt_kprintf("res 默认系数写入完成 \n");
}


/*电阻校准*/
void SBC_ResCal(void)
{
    uint8_t i;
    uint8_t result;
    float v1=0.0,v2=0.0;//电阻两端的实测电压

    rt_kprintf("\n\n-----------------------------------   两线电阻校验   ----------------------------------\n\n");
    for(i=0;i<RES_RANGE_NUM;i++)
    {
        ResCalFactor[i].initflag=0;
        result=1;
        if(ResCalInfo[i].flag==BC_NCAL) continue;
        
        /*选择测试档位*/
        RES_SelectRange((eResRange)ResCalInfo[i].Range);
        rt_thread_delay(MSec(20)); //对于不同档位，此处延时时间不同，测试出来的值不同
//        TestRangeInfo();
        /*电阻1测试*/
        Port_Connect(ResCalInfo[i].R1HPin-1,VOPCUR_CON);
        Port_Connect(ResCalInfo[i].R1LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20)); //对于不同电阻值，此处延时时间不同，测试出来的值不同
        v1=ADC3_GetAvgVolt(32);
        BC_Debug();
        Port_Disconnect(ResCalInfo[i].R1HPin-1,VOPCUR_CON);
        Port_Disconnect(ResCalInfo[i].R1LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20));
        
        /*电阻2测试*/
        Port_Connect(ResCalInfo[i].R2HPin-1,VOPCUR_CON);
        Port_Connect(ResCalInfo[i].R2LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20));
        v2=ADC3_GetAvgVolt(32);
        BC_Debug();
        
        /*打印相关信息*/
        rt_kprintf("Range=%s , ",ResCalInfo[i].RangeName); //档位名称
        rt_kprintf("%s(%.3f Ohm) volt=%.3f[%.3f,%.3f] , ",ResCalInfo[i].R1Name,ResCalInfo[i].Res1,v1,ResCalInfo[i].V1Min,ResCalInfo[i].V1Max); //电阻1信息
        rt_kprintf("%s(%.3f Ohm) volt=%.3f[%.3f,%.3f] , ",ResCalInfo[i].R2Name,ResCalInfo[i].Res2,v2,ResCalInfo[i].V2Min,ResCalInfo[i].V2Max); //电阻2信息
        
        /*判断电压值是否在范围内*/
        if(v1<ResCalInfo[i].V1Min || v1>ResCalInfo[i].V1Max) 
        {
            result=0; 
            rt_kprintf("res1 volt is error\n");
        }
        if(v2<ResCalInfo[i].V2Min || v2>ResCalInfo[i].V2Max) 
        {
            result=0; 
            rt_kprintf("res2 volt is error \n");
        }
        if(result==1)
        {
            ResCalVolt[i].V1=v1;
            ResCalVolt[i].V2=v2;
            EEPROM_Wrf(EPA_B_RES_CAL+i*8,v1);
            EEPROM_Wrf(EPA_B_RES_CAL+4+i*8,v2);
        }
        else
        {
            rt_kprintf("%s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        rt_kprintf("\n");
        
        /*关闭此档位*/
        Port_Disconnect(ResCalInfo[i].R2HPin-1,VOPCUR_CON);
        Port_Disconnect(ResCalInfo[i].R2LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20));
        RES_SelectRange(RES_RANGE_CLOSE);
    }
    rt_kprintf("-----------------------------------   两线电阻校验结束   ----------------------------------\n\n");
}

/*两线电阻自检*/
void SBC_ResSelfTest(void)
{
    uint8_t i;
    float testvalue=0.0,limitmin=0.0,limitmax=0.0;
   
    rt_kprintf("\n\n-----------------------------------   两线电阻自检   ----------------------------------\n\n");
    for(i=0;i<RES_SELF_TEST_NUM;i++)
    {
        if(ResSelfTestInfo[i].flag==BC_NSELF_TEST) continue;
        
        /*设置门限*/
        limitmin=ResSelfTestInfo[i].Res*0.9f;
        limitmax=ResSelfTestInfo[i].Res*1.1f;
        
        /*选择测试档位*/
        RES_SelectRange((eResRange)ResSelfTestInfo[i].Range);
        rt_thread_delay(MSec(20)); //对于不同档位，此处延时时间不同，测试出来的值不同
        
        /*电阻测试*/
        Port_Connect(ResSelfTestInfo[i].HPin-1,VOPCUR_CON);
        Port_Connect(ResSelfTestInfo[i].LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20)); //对于不同电阻值，此处延时时间不同，测试出来的值不同
        RES_Measure(Ohm,50,&testvalue);
        
        /*打印相关信息*/
        rt_kprintf("%s(%f Ohm) Res=%f[%f,%f] ",ResSelfTestInfo[i].Name,ResSelfTestInfo[i].Res,testvalue,limitmin,limitmax);
        rt_kprintf("ADC=%fV CalV1=%f CalV2=%f\n",ADC3_GetAvgVolt(32),ResCalVolt[ResSelfTestInfo[i].Range].V1,ResCalVolt[ResSelfTestInfo[i].Range].V2);
        
        /*判断门限*/
        if(testvalue<limitmin || testvalue>limitmax) 
        {
            rt_kprintf("Res is error . %s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        
        Port_Disconnect(ResSelfTestInfo[i].HPin-1,VOPCUR_CON);
        Port_Disconnect(ResSelfTestInfo[i].LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(10));
        
        /*关闭此档位*/
        RES_SelectRange(RES_RANGE_CLOSE);
    }
    
    rt_kprintf("-----------------------------------   两线电阻自检结束   ----------------------------------\n\n");    
}

FINSH_FUNCTION_EXPORT(SetResCalDefault,SetResCalDefault())

