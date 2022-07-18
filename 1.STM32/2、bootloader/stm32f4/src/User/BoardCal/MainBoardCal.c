/*
 * 缩略词含义：
 * MBC_:Main board calibration （主板校准）
 * MBST_:main board selftest （主板自检）
 */
#include "stdtest_common.h"
extern uint8_t BC_PauseFlag; //暂停标志位  1：暂停运行 0：继续运行 

/*主板电源自检*/
#define MBST_POWER_TEST(iDivisor)   \
    {                               \
        uint8_t flag=0;             \
        while(1)                    \
        {                           \
            adcvalue=ADC3_GetAvgVolt(32);/*读取ADC值*/                        \
            testvalue=adcvalue*iDivisor;/*根据分压电阻得到测试出来的电源电压*/  \
            if(flag==0) rt_kprintf("%s = %.3fV [%.3f,%.3f] (ADC=%.3fV)\n",itemname,testvalue,limitmin,limitmax,adcvalue); \
            if(testvalue<limitmin || testvalue>limitmax)				       \
            {														           \
                if(flag==0)                                                    \
                {                                                               \
                    flag=1;                                                    \
                    BC_PauseFlag=BC_PAUSE;                                     \
                    rt_kprintf("error:%s出错, %s\n",itemname,BC_CONTINE_TIPS); \
                }                               \
            }                                   \
            else                                \
            {                                   \
                break;                          \
            }                                   \
            rt_thread_delay(MSec(100));         \
            if(BC_PauseFlag==BC_CONTINE) break; \
        }                                       \
    }    
    
    
void MBST_Power(void)
{
	float limitminfactor=0.95f,limitmaxfactor=1.05f;
    float r1=510.0f;//分压电阻1 (KOhm)	: 接电源端
	float r2=100.0f;//分压电阻2 (KOhm)	：接GND端
	float limitmin=0.0,limitmax=0.0,testvalue=0.0,adcvalue=0.0f; //门限最小值，门限最大值，测试值，ADC的值
	char itemname[32]={0};

    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_VO_COM);
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_12V_P);
    
	/*+12V电源自检*/
	snprintf(itemname,sizeof(itemname),"+12V 自检");
	limitmin=12.0f*limitminfactor;
	limitmax=12.0f*limitmaxfactor;
	r1=510.0f;//分压电阻1 (KOhm)	: 接电源端
	r2=100.0f;//分压电阻2 (KOhm)	：接GND端
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();
    
// 	/*-12V电源自检*/
//	snprintf(itemname,sizeof(itemname),"-12V 自检");
//	limitmin=12.0f*limitminfactor;
//	limitmax=12.0f*limitmaxfactor;
//    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_12V_N);
//	rt_thread_delay(MSec(50));
//	MBST_POWER_TEST(10.0f);
//    BC_Debug();

	/*+3V电源自检*/
	snprintf(itemname,sizeof(itemname),"+3V 自检");
	limitmin=3.0f*limitminfactor;
	limitmax=3.0f*limitmaxfactor;
	r1=510.0f;//分压电阻1 (KOhm)	: 接电源端
	r2=510.0f;//分压电阻2 (KOhm)	：接GND端
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_3V);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

	/*+3.3V电源自检*/
	snprintf(itemname,sizeof(itemname),"+3.3V 自检");
	limitmin=3.3f*limitminfactor;
	limitmax=3.3f*limitmaxfactor;
	r1=510.0f;//分压电阻1 (KOhm)	: 接电源端
	r2=510.0f;//分压电阻2 (KOhm)	：接GND端
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_3V3);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

	/*+7V电源自检*/
	snprintf(itemname,sizeof(itemname),"+7V 自检");
	limitmin=7*limitminfactor;
	limitmax=7*limitmaxfactor;
	r1=510.0f;//分压电阻1 (KOhm)	: 接电源端
	r2=100.0f;//分压电阻2 (KOhm)	：接GND端
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_7V_P);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

	/*+5V电源自检*/
	snprintf(itemname,sizeof(itemname),"+5V 自检");
	limitmin=5*limitminfactor;
	limitmax=5*limitmaxfactor;
	r1=510.0f;//分压电阻1 (KOhm)	: 接电源端
	r2=100.0f;//分压电阻2 (KOhm)	：接GND端
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_5V_P);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_CLOSE);
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_CLOSE);
	rt_kprintf("主板电源自检完成\n");
}

/*主板电阻自检：主板上有一个1Kohm的电阻*/
void MBST_Res(void)
{
	float limitmin=0.0,limitmax=0.0,testvalue=0.0,adcvalue=0; //门限最小值，门限最大值，测试值
	char itemname[32]={0};
	
	snprintf(itemname,sizeof(itemname),"主板1Kohm电阻自检");

    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_VO_COM);
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_SELFTEST_R);
	RES_SelectRange(RES_RANGE_1KR);
	rt_thread_delay(MSec(50));
	
	limitmin=1000*0.8;
	limitmax=1000*1.2;
    adcvalue=ADC3_GetAvgVolt(32);
	testvalue=adcvalue/ADC1GainValue/CC_GetCurrentValue()*1000;	
	rt_kprintf("%s = %.3f Ohm [%.3f,%.3f] ADC=%.3f V\n",itemname,testvalue,limitmin,limitmax,adcvalue);   
	if(testvalue<limitmin || testvalue>limitmax)							        
	{																		
		rt_kprintf("error:%s出错, %s\n",itemname,BC_CONTINE_TIPS);			
		BC_PauseCmd(BC_PAUSE);												
	}

    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_CLOSE);
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_CLOSE);
	RES_SelectRange(RES_RANGE_CLOSE);
}

void MBST_ADCCal(void)
{
    extern float ADC3GetRealVolt(void);

    float limitmin=0.0,limitmax=0.0,testvalue=0.0; //门限最小值，门限最大值，测试值
	char itemname[32]="ADC 校准";
    uint16_t i;
    
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_AGND); 
	rt_thread_delay(MSec(50));
    
    limitmin=0.05;
	limitmax=0.2;
    
    for(i=0;i<1000;i++) testvalue += ADC3GetRealVolt();
    testvalue/=1000;
	rt_kprintf("%s = %f V [%.3f,%.3f]\n",itemname,testvalue,limitmin,limitmax);   
	if(testvalue<limitmin || testvalue>limitmax)							        
	{																		
		rt_kprintf("error:%s出错, %s\n",itemname,BC_CONTINE_TIPS);			
		BC_PauseCmd(BC_PAUSE);												
	}
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_CLOSE);
    
}


