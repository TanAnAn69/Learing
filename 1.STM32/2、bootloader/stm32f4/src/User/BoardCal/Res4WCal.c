/*
 * 缩略词含义：
 * SBC_:switch board calibration （swithc板校准）
 * SBST_:switch board selftest （swithc板自检）
 */
#include "stdtest_common.h"
extern uint16_t BC_EepromAddr;

#define EPA_B_RES4W_CAL (BC_EepromAddr+80)

typedef struct
{ 
	float V1;
	float V2;
}sRes4WCalVolt;//电阻校验电压

/*电阻校准系数*/
typedef struct
{
	uint8_t initflag; 
    float k; 
	float b;
}sRes4WCalFactor;

typedef struct
{
	const char *RangeName;//档位名称
    uint16_t Range; //需要校准的档位

    const char *R1Name; //电阻1名称
	uint16_t R1HPin1; //电阻1的高引脚1 接VO
	uint16_t R1HPin2; //电阻1的高引脚2 接CUR+
	uint16_t R1LPin1; //电阻1的低引脚1 接COM
	uint16_t R1LPin2; //电阻1的低引脚2 接CUR-
    float Res1;     //电阻1的阻值
    
    const char *R2Name; //电阻2名称    
	uint16_t R2HPin1; //电阻2的高引脚1 接VO
	uint16_t R2HPin2; //电阻2的高引脚2 接CUR+
	uint16_t R2LPin1; //电阻2的低引脚1 接COM
	uint16_t R2LPin2; //电阻2的低引脚2 接CUR-
    float Res2;      //电阻2的阻值

    float V1Min; //V1最小门限
    float V1Max; //V1最大门限
    float V2Min; //V2最小门限
    float V2Max; //V2最大门限

	uint16_t flag;    //是否需要校验的标志位
}sRes4WCalInfo;

/*四线电阻自检时需要用到的相关信息*/
typedef struct
{
    const char *Name; //电阻名称
    uint16_t Range; //测试此电阻的档位
	uint16_t HPin1; //电阻的高引脚1
	uint16_t HPin2; //电阻的高引脚2
	uint16_t LPin1; //电阻的低引脚1
	uint16_t LPin2; //电阻的低引脚2
    float Res;     //电阻的阻值
	uint16_t flag;    //是否需要自检
}sRes4WSelfTestInfo;



static uint8_t Res4WCalFlag=0; //res校验电压是否正确  0:Res校验电压错误  1:res校验电压正确
static uint8_t Res4WCalInitFlag=0; //res校验电压是否初始化，0：电压未初始化  1：电压已经初始化
static sRes4WCalVolt Res4WCalVolt[RES4W_RANGE_NUM]={0.0};
static sRes4WCalFactor Res4WCalFactor[RES4W_RANGE_NUM];

static const sRes4WCalInfo Res4WCalInfo[RES4W_RANGE_NUM]=
{
    /*RangeName,Range  ;  R1Name,R1HPin1,R1HPin2,R1LPin1,R1LPin2,Res1  ;  R2Name,R2HPin,R2LPin,Res2  ;  V1Min,V1Max,V2Min,V2Max  ;  unit,flag*/
    {"RES4W_RANGE_100mR",RES4W_RANGE_100mR,"R84",0,0,0,0,0.1,"R384",0,0,0,0,0.01    ,1.489,2.481,0.05,0.248,BC_CAL},
    {"RES4W_RANGE_1R",RES4W_RANGE_1R,"R49",0,0,0,0,1,"R84",0,0,0,0,0.1    ,1.179,1.964,0.118,0.196,BC_CAL},
    {"RES4W_RANGE_10R",RES4W_RANGE_10R,"R12",1,3,2,4,2.21,"R11",5,7,6,8,4.99    ,0.451,0.498,1.018,1.125,BC_CAL},
    {"RES4W_RANGE_100R",RES4W_RANGE_100R,"R2",9,11,10,12,33,"R3",13,15,14,16,75    ,0.624,0.689,1.417,1.566,BC_CAL},
    {"RES4W_RANGE_1KR",RES4W_RANGE_1KR,"R1",17,19,18,20,330,"R4",21,23,22,24,750    ,0.624,0.689,1.417,1.566,BC_CAL},
    {"RES4W_RANGE_10KR",RES4W_RANGE_10KR,"R6",25,27,26,28,3300,"R5",29,31,30,32,6200    ,0.624,0.689,1.172,1.295,BC_CAL},
    {"RES4W_RANGE_100KR",RES4W_RANGE_100KR,"R7",33,35,34,36,30000,"R8",37,39,38,40,73200    ,0.754,0.922,1.2,1.467,BC_CAL},
    {"RES4W_RANGE_1MR",RES4W_RANGE_1MR,"R10",41,43,42,44,191000,"R9",45,47,46,48,470000    ,0.564,0.689,0.988,1.207,BC_CAL},
};

/*用于四线自检的电阻信息*/
#define RES4W_SELF_TEST_NUM 14
static const sRes4WSelfTestInfo Res4WSelfTestInfo[RES4W_SELF_TEST_NUM]=
{    
    /*Name,Range,HPin,LPin,unit,flag*/
    {"R19", RES4W_RANGE_100mR,49,51,50,52,5, BC_SELF_TEST},
    {"R12", RES4W_RANGE_100mR,53,55,54,56,50,  BC_SELF_TEST},
    {"R11", RES4W_RANGE_10R,    5, 7, 6, 8, 8.3,  BC_SELF_TEST},
    {"R2",  RES4W_RANGE_100R,  9, 11,10,12,50.9, BC_SELF_TEST},
    {"R3",  RES4W_RANGE_100R,  13,15,14,16,100,   BC_SELF_TEST},
    {"R1",  RES4W_RANGE_1KR,  17,19,18,20,148.9,BC_SELF_TEST},
    {"R4",  RES4W_RANGE_1KR,  21,23,22,24,510.3, BC_SELF_TEST},
    {"R6",  RES4W_RANGE_10KR,  25,27,26,28,1.498 ,BC_SELF_TEST},
    {"R5",  RES4W_RANGE_10KR,  29,31,30,32,9.12,  BC_SELF_TEST},
    {"R7",  RES4W_RANGE_100KR,33,35,34,36,20.0,  BC_SELF_TEST},
    {"R8",  RES4W_RANGE_100KR,37,39,38,40,55.96, BC_SELF_TEST},
    {"R10", RES4W_RANGE_1MR, 41,43,42,44,100.59,BC_SELF_TEST},
    {"R9",  RES4W_RANGE_1MR, 45,47,46,48,570.18,BC_SELF_TEST},
};

/*读取Res4W校验系数*/
void SBC_GetRes4WCalFactor(float *k,float *b)
{
    float res1,res2,v1,v2;
    FWAssertError(Res4WCalInitFlag,"Res4W 校验电压未初始化!\n");
    FWAssertError(Res4WCurRange<RES4W_RANGE_NUM,"Res4W Range error = %d! \n",Res4WCurRange);
    FWAssertError(Res4WCalFlag,"Res4W 校验电压存在错误，请重新校验!\n");
    FWAssertError(Res4WCalInfo[Res4WCurRange].flag==BC_CAL,"请不要使用未校验的档位 = %s\n",Res4WCalInfo[Res4WCurRange].RangeName);

    if(Res4WCalFactor[Res4WCurRange].initflag==0)
    {
        res1=Res4WCalInfo[Res4WCurRange].Res1;
        res2=Res4WCalInfo[Res4WCurRange].Res2;
        v1=Res4WCalVolt[Res4WCurRange].V1;
        v2=Res4WCalVolt[Res4WCurRange].V2;
        
//        TestRangeInfo();
//        rt_kprintf("res1=%.3f,res2=%.3f,v1=%.3f,v2=%.3f\n",res1,res2,v1,v2);
        /*计算系数*/
        if(Res4WTestMethod==RES4W_CC_TEST || Res4WTestMethod==RES4W_CP_TEST)
        {
            Res4WCalFactor[Res4WCurRange].k=(res1-res2)/(v1-v2); //k=(y1-y2)/(x1-x2)
            Res4WCalFactor[Res4WCurRange].b=res1-Res4WCalFactor[Res4WCurRange].k*v1;//b=y1-kx1
        }
        else if(Res4WTestMethod==RES4W_CV_TEST)
        {
            Res4WCalFactor[Res4WCurRange].b=res1*res2*(v2-v1)/(v1*res2-v2*res1);
            Res4WCalFactor[Res4WCurRange].k=v1/res1*(Res4WCalFactor[Res4WCurRange].b+res1);
        }
        else
        {
            FWAssertError(0,"Res4WTestMethod=%d !\n",Res4WTestMethod);
        }
        
        Res4WCalFactor[Res4WCurRange].initflag=1;
    }
    
    *k=Res4WCalFactor[Res4WCurRange].k;
    *b=Res4WCalFactor[Res4WCurRange].b;
}

/*Res4W 校验电压初始化*/
void SBC_Res4WCalInit(void)
{
    uint8_t i;
    float v1,v2;//电压
    
    PRINT_INIT_START();
    Res4WCalFlag=1;
    for(i=0;i<RES4W_RANGE_NUM;i++)
    {
        if(Res4WCalInfo[i].flag==BC_NCAL) continue;
        
        /*从eeprom中读取电压*/
        v1=EEPROM_Rdf(EPA_B_RES4W_CAL+i*8);
        v2=EEPROM_Rdf(EPA_B_RES4W_CAL+4+i*8);
        
        /*打印电压信息*/
        rt_kprintf("Res4W cal:%s , v1=%.3f[%.3f,%.3f] add=%d , v2=%.3f[%.3f,%.3f] add=%d\n",Res4WCalInfo[i].RangeName,
                    v1,Res4WCalInfo[i].V1Min,Res4WCalInfo[i].V1Max,EPA_B_RES4W_CAL+i*8,
                    v2,Res4WCalInfo[i].V2Min,Res4WCalInfo[i].V2Max,EPA_B_RES4W_CAL+4+i*8);
        /*判断电压是否在门限内*/
        if(v1>=Res4WCalInfo[i].V1Min && v1<=Res4WCalInfo[i].V1Max && v2>=Res4WCalInfo[i].V2Min && v2<=Res4WCalInfo[i].V2Max)
        {
            Res4WCalVolt[i].V1=v1;
            Res4WCalVolt[i].V2=v2;
        }
        else
        {
            Res4WCalFlag=0;//校验电压错误
            rt_kprintf("--------档位%s的校验电压错误----------\n\n",Res4WCalInfo[i].RangeName);
        }
    }
    Res4WCalInitFlag=1;
    PRINT_INIT_OK();
}

/*设置为默认的校验电压：默认的校验电压为门限的中间值
 注：此功能只有在需要测试电阻而手边又没有校验好的板子时使用*/
static void SetRes4WCalDefault(void)
{
    uint8_t i=0;
    for(i=0;i<RES4W_RANGE_NUM;i++)
    {
        Res4WCalVolt[i].V1=(Res4WCalInfo[i].V1Min+Res4WCalInfo[i].V1Max)/2;
        Res4WCalVolt[i].V2=(Res4WCalInfo[i].V2Min+Res4WCalInfo[i].V2Max)/2;
        EEPROM_Wrf(EPA_B_RES4W_CAL+i*8,Res4WCalVolt[i].V1);
        EEPROM_Wrf(EPA_B_RES4W_CAL+4+i*8,Res4WCalVolt[i].V2);
        /*打印电压信息*/
        rt_kprintf("Res4W cal:%s , v1=%.3f[%.3f,%.3f] add=%d , v2=%.3f[%.3f,%.3f] add=%d\n",
                   Res4WCalInfo[i].RangeName,Res4WCalVolt[i].V1,Res4WCalInfo[i].V1Min,Res4WCalInfo[i].V1Max,EPA_B_RES4W_CAL+i*8,
                   Res4WCalVolt[i].V2,Res4WCalInfo[i].V2Min,Res4WCalInfo[i].V2Max,EPA_B_RES4W_CAL+4+i*8);
    }
    Res4WCalFlag=1;
    rt_kprintf("res4W 默认电压写入完成 \n");
}


/*四线电阻校准*/
void SBC_Res4WCal(void)
{
    uint8_t i;
    uint8_t result;
    float v1=0.0,v2=0.0;
    
    rt_kprintf("\n\n-----------------------------------   四线电阻校验   ----------------------------------\n\n");
    for(i=0;i<RES4W_RANGE_NUM;i++)
    {
        result=1;
        if(Res4WCalInfo[i].flag==BC_NCAL) continue;
        
        /*选择测试档位*/
        RES4W_SelectRange((eRes4WRange)Res4WCalInfo[i].Range);
//        TestRangeInfo();
        rt_thread_delay(MSec(20)); //对于不同档位，此处延时时间不同，测试出来的值不同
        if(Res4WCalInfo[i].Range==RES4W_RANGE_100mR || Res4WCalInfo[i].Range==RES4W_RANGE_1R) //mOhm档位的电阻采用比较法测试，与恒压/恒流法不同
        {
            Port_Connect(Res4WCalInfo[2].R1HPin2-1,PCUR_CON); //注意：由于CPU板上没有途径将cur+和cur-短路，此处通过开关板上的小电阻将cur+和cur-接在一起
            Port_Connect(Res4WCalInfo[2].R1LPin2-1,NCUR_CON); //      以便测量主板上的比较电阻
            if(Res4WCalInfo[i].Range==RES4W_RANGE_100mR)
            {
                VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_COMPARE_RES_0R100);
                rt_thread_delay(MSec(20));
                v1=ADC3_GetAvgVolt(32);
                BC_Debug();
                VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_COMPARE_RES_0R010);
                rt_thread_delay(MSec(20));
                v2=ADC3_GetAvgVolt(32);
                BC_Debug();
            }
            else if(Res4WCalInfo[i].Range==RES4W_RANGE_1R)
            {
                VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_COMPARE_RES_1R);
                rt_thread_delay(MSec(20));
                v1=ADC3_GetAvgVolt(32);
                BC_Debug();
                VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_COMPARE_RES_0R100);
                rt_thread_delay(MSec(20));
                v2=ADC3_GetAvgVolt(32);
                BC_Debug();
            }
            else
            {
                FWAssertError(0,"Range error !\n");
            }
            Port_Disconnect(Res4WCalInfo[2].R1HPin2-1,PCUR_CON); 
            Port_Disconnect(Res4WCalInfo[2].R1LPin2-1,NCUR_CON); 
            VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_CLOSE);
        }
        else
        {
            /*电阻1测试*/
            Port_Connect(Res4WCalInfo[i].R1HPin1-1,VO_CON);
            Port_Connect(Res4WCalInfo[i].R1HPin2-1,PCUR_CON);
            Port_Connect(Res4WCalInfo[i].R1LPin1-1,COM_CON);
            Port_Connect(Res4WCalInfo[i].R1LPin2-1,NCUR_CON);
            rt_thread_delay(MSec(20)); //对于不同电阻值，此处延时时间不同，测试出来的值不同
            v1=ADC3_GetAvgVolt(32);
            BC_Debug();
            Port_Disconnect(Res4WCalInfo[i].R1HPin1-1,VO_CON);
            Port_Disconnect(Res4WCalInfo[i].R1HPin2-1,PCUR_CON);
            Port_Disconnect(Res4WCalInfo[i].R1LPin1-1,COM_CON);
            Port_Disconnect(Res4WCalInfo[i].R1LPin2-1,NCUR_CON);
            rt_thread_delay(MSec(20));
            
            /*电阻2测试*/
            Port_Connect(Res4WCalInfo[i].R2HPin1-1,VO_CON);
            Port_Connect(Res4WCalInfo[i].R2HPin2-1,PCUR_CON);
            Port_Connect(Res4WCalInfo[i].R2LPin1-1,COM_CON);
            Port_Connect(Res4WCalInfo[i].R2LPin2-1,NCUR_CON);
            rt_thread_delay(MSec(20));
            v2=ADC3_GetAvgVolt(32);
            BC_Debug();
        }

        Res4WCalVolt[i].V1=v1;
        Res4WCalVolt[i].V2=v2;

        /*打印相关信息*/
        rt_kprintf("Range=%s , ",Res4WCalInfo[i].RangeName); //档位名称
        rt_kprintf("%s(%.3f) volt=%.3f , ",Res4WCalInfo[i].R1Name,Res4WCalInfo[i].Res1,v1); //电阻1信息
        rt_kprintf("%s(%.3f) volt=%.3f , ",Res4WCalInfo[i].R2Name,Res4WCalInfo[i].Res2,v2); //电阻2信息
        rt_kprintf("v1=%.3f[%.3f,%.3f] , ",Res4WCalVolt[i].V1,Res4WCalInfo[i].V1Min,Res4WCalInfo[i].V1Max); //k电压信息
        rt_kprintf("v2=%.3f[%.3f,%.3f] \n",Res4WCalVolt[i].V2,Res4WCalInfo[i].V2Min,Res4WCalInfo[i].V2Max); //b电压信息
        
        /*判断校验电压是否在范围内*/
        if(Res4WCalVolt[i].V1<Res4WCalInfo[i].V1Min || Res4WCalVolt[i].V1>Res4WCalInfo[i].V1Max) 
        {
            result=0; 
            rt_kprintf("V1 is error\n");
        }
        if(Res4WCalVolt[i].V2<Res4WCalInfo[i].V2Min || Res4WCalVolt[i].V2>Res4WCalInfo[i].V2Max) 
        {
            result=0; 
            rt_kprintf("V2 is error \n");
        }
        if(result==1)
        {
            EEPROM_Wrf(EPA_B_RES4W_CAL+i*8,Res4WCalVolt[i].V1);
            EEPROM_Wrf(EPA_B_RES4W_CAL+4+i*8,Res4WCalVolt[i].V2);
        }
        else
        {
            rt_kprintf("%s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        
        /*关闭此档位*/
        if(Res4WCalInfo[i].Range != RES4W_RANGE_100mR && Res4WCalInfo[i].Range!=RES4W_RANGE_1R)
        {
            Port_Disconnect(Res4WCalInfo[i].R2HPin1-1,VO_CON);
            Port_Disconnect(Res4WCalInfo[i].R2HPin2-1,PCUR_CON);
            Port_Disconnect(Res4WCalInfo[i].R2LPin1-1,COM_CON);
            Port_Disconnect(Res4WCalInfo[i].R2LPin2-1,NCUR_CON);
        }
        rt_thread_delay(MSec(20));
        RES4W_SelectRange(RES4W_RANGE_CLOSE);
    }
    rt_kprintf("-----------------------------------   四线电阻校验结束   ----------------------------------\n\n");
}

/*电阻自检*/
void SBC_Res4WSelfTest(void)
{
    uint8_t i;
    float testvalue=0.0,limitmin=0.0,limitmax=0.0;
    
    rt_kprintf("\n\n-----------------------------------   四线电阻自检   ----------------------------------\n\n");
    for(i=0;i<RES4W_SELF_TEST_NUM;i++)
    {
        if(Res4WSelfTestInfo[i].flag==BC_NSELF_TEST) continue;
        
        limitmin=Res4WSelfTestInfo[i].Res*(float)0.95;
        limitmax=Res4WSelfTestInfo[i].Res*(float)1.05;
        
        /*选择测试档位*/
        RES4W_SelectRange((eRes4WRange)Res4WSelfTestInfo[i].Range);
        rt_thread_delay(MSec(20)); 
        
        /*电阻1测试*/
        Port_Connect(Res4WSelfTestInfo[i].HPin1-1,VO_CON);
        Port_Connect(Res4WSelfTestInfo[i].HPin2-1,PCUR_CON);
        Port_Connect(Res4WSelfTestInfo[i].LPin1-1,COM_CON);
        Port_Connect(Res4WSelfTestInfo[i].LPin2-1,NCUR_CON);
        rt_thread_delay(MSec(20)); 
        RES4W_Measure(Ohm,50,&testvalue);
                
        /*打印相关信息*/
        rt_kprintf("%s(%.3f%s) Res=%.3f[%.3f,%.3f] ",Res4WSelfTestInfo[i].Name,Res4WSelfTestInfo[i].Res,testvalue,limitmin,limitmax);
        rt_kprintf("ADC=%.3fV v1=%.3f v2=%.3f\n",ADC3_GetAvgVolt(32),Res4WCalVolt[Res4WSelfTestInfo[i].Range].V1,Res4WCalVolt[Res4WSelfTestInfo[i].Range].V2);
        
        /*判断门限*/
        if(testvalue<limitmin || testvalue>limitmax) 
        {
            rt_kprintf("Res is error . %s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        
        /*关闭开关*/
        Port_Disconnect(Res4WSelfTestInfo[i].HPin1-1,VO_CON);
        Port_Disconnect(Res4WSelfTestInfo[i].HPin2-1,PCUR_CON);
        Port_Disconnect(Res4WSelfTestInfo[i].LPin1-1,COM_CON);
        Port_Disconnect(Res4WSelfTestInfo[i].LPin2-1,NCUR_CON);
        RES4W_SelectRange(RES4W_RANGE_CLOSE);
        rt_thread_delay(MSec(20));
    }
    rt_kprintf("-----------------------------------   四线电阻自检结束   ----------------------------------\n\n");
}

FINSH_FUNCTION_EXPORT(SetRes4WCalDefault,SetRes4WCalDefault())

