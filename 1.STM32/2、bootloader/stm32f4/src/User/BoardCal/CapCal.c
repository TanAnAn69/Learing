/*
 * 缩略词含义：
 * SBC_:switch board calibration （swithc板校准）
 * SBST_:switch board selftest （swithc板自检）
 */
#include "stdtest_common.h"
extern uint8_t CAPCR_SelectCalRes(eCapCRCalRes iCapCRCalRes);
extern uint16_t BC_EepromAddr;

#define EPA_B_CAP_CAL (BC_EepromAddr+200)


/*主板上和校准板上电阻两端的电压*/ 
typedef struct
{
	float V1;//主板上校准电阻1两端电压
    float V2;//主板上校准电阻2两端电压
}sCapCalVolt;

/*电阻校准系数*/ 
typedef struct
{
	uint8_t initflag;
    float R1;
    float R2;
    float V1; 
	float V2;
}sCapCalFactor;

/*电容校验时需要用到的相关信息：全部以nF为单位*/
typedef struct
{
	const char *RangeName;//档位名称
    uint16_t Range; //需要校准的档位

    const char *R1Name; //主板：电阻1名称
    float Res1;     //主板：电阻1的阻值
    
    const char *R2Name; //主板：电阻2名称    
    float Res2;      //主板：电阻2的阻值

    float V1Min;//主板：电阻1两端电压最小值
    float V1Max;//主板：电阻1两端电压最大值
    float V2Min;//主板：电阻2两端电压最小值
    float V2Max;//主板：电阻2两端电压最大 值
	uint16_t flag;    //是否需要校验的标志位
}sCapCalInfo;

/*电容自检时需要用到的相关信息：全部以nF为单位*/
typedef struct
{
    const char *Name; //电容名称
    uint16_t Range; //测试此电容的档位
	uint16_t HPin; //电容的高引脚
	uint16_t LPin; //电容的低引脚
    float Cap;     //电容的阻值
	uint16_t flag; //是否需要自检
}sCapSelfTestInfo;

/*用于两线校准时的电容信息*/
static uint8_t CapCalFlag=0; //res校验系数是否正确  0:Cap校验系数错误  1:res校验系数正确
static uint8_t CapCalInitFlag=0; //res校验系数是否初始化，0：系数未初始化  1：系数已经初始化
static sCapCalVolt CapCalVolt[CAP_RANGE_NUM];
static sCapCalFactor CapCalFactor[CAP_RANGE_NUM];
static const sCapCalInfo CapCalInfo[CAP_RANGE_NUM]=
{
    /*RangeName      Range             R1Name,Res1,R2Name,Res2 ; V1Min,V1Max,V2Min,V2Max;flag*/
    {"CAP_RANGE_100pF",CAP_RANGE_100pF,"R6",100000,"R8",154000    ,0.38,0.78,0.26,0.66,BC_CAL},
    {"CAP_RANGE_500pF",CAP_RANGE_500pF,"R6",100000,"R8",154000    ,0.54,0.94,0.13,0.53,BC_CAL},
    {"CAP_RANGE_5nF",CAP_RANGE_5nF,    "R53",10000,"R54",14700    ,0.51,0.91,0.19,0.59,BC_CAL},
    {"CAP_RANGE_50nF",CAP_RANGE_50nF,  "R53",10000,"R54",14700    ,0.50,0.90,0.18,0.58,BC_CAL},
//    
//    {"CAP_RANGE_500nF",CAP_RANGE_500nF,"R7",33,34,30000,"R8",37,38,73200    ,0.76,0.929,1.209,1.478,BC_CAL},
//    {"CAP_RANGE_1uF",CAP_RANGE_1uF,"R10",41,42,191000,"R9",45,46,470000    ,0.568,0.694,0.995,1.216,BC_CAL},
//    {"CAP_RANGE_5uF",CAP_RANGE_5uF,"null",0,0,0,"null",0,0,0    ,0,0,0,0,BC_NCAL},
};

/*用于两线自检的电容信息*/
#define CAP_SELF_TEST_NUM 12
static const sCapSelfTestInfo CapSelfTestInfo[CAP_SELF_TEST_NUM]=
{    
    /*Name,Range,HPin,LPin,unit,flag*/
    {"R12", CAP_RANGE_100pF,    1, 2, 3.33,  BC_SELF_TEST},
    {"R11", CAP_RANGE_100pF,    5, 6, 8.3,   BC_SELF_TEST},
    {"R2",  CAP_RANGE_500pF,  9, 10,50.9,  BC_SELF_TEST},
    {"R3",  CAP_RANGE_500pF,  13,14,100,   BC_SELF_TEST},
    {"R1",  CAP_RANGE_5nF,  17,18,148.9, BC_SELF_TEST},
    {"R4",  CAP_RANGE_5nF,  21,22,510.3, BC_SELF_TEST},
    {"R6",  CAP_RANGE_50nF,  25,26,1.498, BC_SELF_TEST},
    {"R5",  CAP_RANGE_50nF,  29,30,9.12,  BC_SELF_TEST},
    {"R7",  CAP_RANGE_500nF,33,34,20,    BC_SELF_TEST},
    {"R8",  CAP_RANGE_500nF,37,38,55.96, BC_SELF_TEST},
    {"R10", CAP_RANGE_1uF, 41,42,100.59,BC_SELF_TEST},
    {"R9",  CAP_RANGE_1uF, 45,46,570.18,BC_SELF_TEST},
};

/*获取取校验系数*/
void SBC_GetCapCalFactor(float *iR1,float *iR2,float *iV1,float *iV2)
{
    float r1,r2;//校准板上两个电阻
    float v1,v2;//校准板上两个电阻对应的电压
    
    FWAssertError(CapCalInitFlag,"Cap 校验系数未初始化!\n");
    FWAssertError(CapCurRange<CAP_RANGE_NUM,"Range error!\n");
    FWAssertError(CapCalFlag,"Cap 校验系数存在错误，请重新校验!\n");
    FWAssertError(CapCalInfo[CapCurRange].flag==BC_CAL,"请不要使用未校验的档位 = %s\n",CapCalInfo[CapCurRange].RangeName);
    
    if(CapCalFactor[CapCurRange].initflag==0)
    {
        v1=CapCalVolt[CapCurRange].V1;
        v2=CapCalVolt[CapCurRange].V2;
        
        /*计算系数*/
        if(CapTestMethod==CAP_CR_TEST)
        {
            CapCalFactor[CapCurRange].R1=r1;
            CapCalFactor[CapCurRange].R2=r2;
            CapCalFactor[CapCurRange].V1=v1;
            CapCalFactor[CapCurRange].V2=v2;
        }
        else if(CapTestMethod==CAP_CD_TEST)
        {
        }
        else
        {
            FWAssertError(0,"CapTestMethod=%d !\n",CapTestMethod);
        }
        CapCalFactor[CapCurRange].initflag=1;
    }
    
    *iR1=CapCalFactor[CapCurRange].R1;
    *iR2=CapCalFactor[CapCurRange].R2;
    *iV1=CapCalFactor[CapCurRange].V1;
    *iV2=CapCalFactor[CapCurRange].V2;
}

/*Cap校验系数初始化*/
void SBC_CapCalInit(void)
{
    uint8_t i;
    float v1,v2,calv1,calv2;
    
    PRINT_INIT_START();
    CapCalFlag=1;
    for(i=0;i<CAP_RANGE_NUM;i++)
    {
        if(CapCalInfo[i].flag==BC_NCAL) continue;
        
        /*从eeprom中读取电容两端的校验电压*/
        calv1=EEPROM_Rdf(EPA_B_CAP_CAL+i*8);
        calv2=EEPROM_Rdf(EPA_B_CAP_CAL+4+i*8);
        
        /*打印系数信息*/
        rt_kprintf("Cap cal:%s V1=.3%f[%.3f,%.3f] add=%d , V2=%.3f[%.3f,%.3f] add=%d\n",
                    CapCalInfo[i].RangeName,
                    v1,CapCalInfo[i].V1Min ,CapCalInfo[i].V1Max,EPA_B_CAP_CAL+i*8,   
                    v2,CapCalInfo[i].V2Min,CapCalInfo[i].V2Max,EPA_B_CAP_CAL+4+i*8);
        /*判断电压是否在门限内*/
        if(v1>=CapCalInfo[i].V1Min && v1<=CapCalInfo[i].V1Max && v2>=CapCalInfo[i].V2Min && v2<=CapCalInfo[i].V2Max)
        {
            CapCalVolt[i].V1=calv1;
            CapCalVolt[i].V2=calv2;
        }
        else
        {
            CapCalFlag=0;//校验系数错误
            rt_kprintf("--------档位%s的校验系数错误----------\n\n",CapCalInfo[i].RangeName);
        }
    }
    CapCalInitFlag=1;
    PRINT_INIT_OK();
}

#include "stdlib_gpio.h"
/*电容校准*/
void SBC_CapCal(void)
{
    uint8_t i,j=0;
    uint8_t result;
    uint16_t delay=300;
    float v1=0.0,v2=0.0;//校准板上电阻两端的电压
    
    rt_kprintf("\n\n-----------------------------------   电容校验   ----------------------------------\n\n");
    for(i=0;i<CAP_RANGE_NUM;i++)
    {
        CapCalFactor[i].initflag=0;
        result=1;
        
        if(CapCalInfo[i].flag==BC_NCAL) continue;

        /*选择测试档位*/
        CAPCR_SelectRange((eCapRange)CapCalInfo[i].Range);
        CURPN_SelectOutputSignal(CURPN_OUTPUT_SIGNAL_CLOSE);
        
        /*主板电阻1测试*/
        if(CapCRCurRangeRes==CAP_CR_RANGE_RES1) CAPCR_SelectCalRes(CAP_CR_CAL_RES1_1);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES2) CAPCR_SelectCalRes(CAP_CR_CAL_RES2_1);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES3) CAPCR_SelectCalRes(CAP_CR_CAL_RES3_1);
        
        CAPCR_NonloadInputDisable();
        rt_thread_delay(MSec(delay));
        v1=ADC3_GetAvgVolt(32);
        BC_Debug();
        CAPCR_NonloadInputEnable();
        
        /*主板电阻2测试*/
        if(CapCRCurRangeRes==CAP_CR_RANGE_RES1) CAPCR_SelectCalRes(CAP_CR_CAL_RES1_2);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES2) CAPCR_SelectCalRes(CAP_CR_CAL_RES2_2);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES3) CAPCR_SelectCalRes(CAP_CR_CAL_RES3_2);
        
        CAPCR_NonloadInputDisable();
        rt_thread_delay(MSec(delay));
        v2=ADC3_GetAvgVolt(32);
        BC_Debug();
        CAPCR_NonloadInputEnable();

        /*打印相关信息*/
        rt_kprintf("Range=%s , ",CapCalInfo[i].RangeName); //档位名称
        rt_kprintf("%s(%.0f Ohm) volt=%.3f[%.3f,%.3f] , ",CapCalInfo[i].R1Name,CapCalInfo[i].Res1,v1,CapCalInfo[i].V1Min,CapCalInfo[i].V1Max); //电容1信息
        rt_kprintf("%s(%.0f Ohm) volt=%.3f[%.3f,%.3f]\n",CapCalInfo[i].R2Name,CapCalInfo[i].Res2,v2,CapCalInfo[i].V2Min,CapCalInfo[i].V2Max); //电容2信息
        
        /*判断电压值是否在范围内*/
        if(v1<CapCalInfo[i].V1Min || v1>CapCalInfo[i].V1Max) 
        {
            result=0; 
            rt_kprintf("res1 volt is error\n");
        }
        if(v2<CapCalInfo[i].V2Min || v2>CapCalInfo[i].V2Max) 
        {
            result=0; 
            rt_kprintf("res2 volt is error \n");
        }
        if(result==1)
        {
            EEPROM_Wrf(EPA_B_CAP_CAL+i*8,v1);
            EEPROM_Wrf(EPA_B_CAP_CAL+4+i*8,v2);
            CapCalFactor[i].initflag=1;
        }
        else
        {
            rt_kprintf("%s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        
        /*关闭此档位*/
        CAPCR_SelectRange(CAP_RANGE_CLOSE);
    }
    rt_kprintf("-----------------------------------   电容校验结束   ----------------------------------\n\n");
}

/*两线电容自检*/
void SBC_CapSelfTest(void)
{
    uint8_t i;
    float testvalue=0.0,limitmin=0.0,limitmax=0.0;
   
    rt_kprintf("\n\n-----------------------------------   电容自检   ----------------------------------\n\n");
    for(i=0;i<CAP_SELF_TEST_NUM;i++)
    {
        if(CapSelfTestInfo[i].flag==BC_NSELF_TEST) continue;
        
        /*设置门限*/
        limitmin=CapSelfTestInfo[i].Cap*0.9f;
        limitmax=CapSelfTestInfo[i].Cap*1.1f;
        
        /*选择测试档位*/
        CAPCR_SelectRange((eCapRange)CapSelfTestInfo[i].Range);
        rt_thread_delay(MSec(20)); //对于不同档位，此处延时时间不同，测试出来的值不同
        
        /*电容测试*/
        Port_Connect(CapSelfTestInfo[i].HPin-1,VOPCUR_CON);
        Port_Connect(CapSelfTestInfo[i].LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20)); //对于不同电容值，此处延时时间不同，测试出来的值不同
        CAP_Measure(nF,50,&testvalue);
        
        /*打印相关信息*/
        rt_kprintf("%s(%f nF) Cap=%f[%f,%f] ",CapSelfTestInfo[i].Name,CapSelfTestInfo[i].Cap,testvalue,limitmin,limitmax);
        rt_kprintf("ADC=%fV CalV1=%f CalV2=%f\n",ADC3_GetAvgVolt(32),CapCalVolt[CapSelfTestInfo[i].Range].V1,CapCalVolt[CapSelfTestInfo[i].Range].V2);
        
        /*判断门限*/
        if(testvalue<limitmin || testvalue>limitmax) 
        {
            rt_kprintf("Cap is error . %s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        
        Port_Disconnect(CapSelfTestInfo[i].HPin-1,VOPCUR_CON);
        Port_Disconnect(CapSelfTestInfo[i].LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(10));
        
        /*关闭此档位*/
        CAPCR_SelectRange(CAP_RANGE_CLOSE);
    }
    
    rt_kprintf("-----------------------------------   电容自检结束   ----------------------------------\n\n");    
}



