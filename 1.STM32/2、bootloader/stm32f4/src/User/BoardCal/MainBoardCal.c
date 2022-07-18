/*
 * ���Դʺ��壺
 * MBC_:Main board calibration ������У׼��
 * MBST_:main board selftest �������Լ죩
 */
#include "stdtest_common.h"
extern uint8_t BC_PauseFlag; //��ͣ��־λ  1����ͣ���� 0���������� 

/*�����Դ�Լ�*/
#define MBST_POWER_TEST(iDivisor)   \
    {                               \
        uint8_t flag=0;             \
        while(1)                    \
        {                           \
            adcvalue=ADC3_GetAvgVolt(32);/*��ȡADCֵ*/                        \
            testvalue=adcvalue*iDivisor;/*���ݷ�ѹ����õ����Գ����ĵ�Դ��ѹ*/  \
            if(flag==0) rt_kprintf("%s = %.3fV [%.3f,%.3f] (ADC=%.3fV)\n",itemname,testvalue,limitmin,limitmax,adcvalue); \
            if(testvalue<limitmin || testvalue>limitmax)				       \
            {														           \
                if(flag==0)                                                    \
                {                                                               \
                    flag=1;                                                    \
                    BC_PauseFlag=BC_PAUSE;                                     \
                    rt_kprintf("error:%s����, %s\n",itemname,BC_CONTINE_TIPS); \
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
    float r1=510.0f;//��ѹ����1 (KOhm)	: �ӵ�Դ��
	float r2=100.0f;//��ѹ����2 (KOhm)	����GND��
	float limitmin=0.0,limitmax=0.0,testvalue=0.0,adcvalue=0.0f; //������Сֵ���������ֵ������ֵ��ADC��ֵ
	char itemname[32]={0};

    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_VO_COM);
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_12V_P);
    
	/*+12V��Դ�Լ�*/
	snprintf(itemname,sizeof(itemname),"+12V �Լ�");
	limitmin=12.0f*limitminfactor;
	limitmax=12.0f*limitmaxfactor;
	r1=510.0f;//��ѹ����1 (KOhm)	: �ӵ�Դ��
	r2=100.0f;//��ѹ����2 (KOhm)	����GND��
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();
    
// 	/*-12V��Դ�Լ�*/
//	snprintf(itemname,sizeof(itemname),"-12V �Լ�");
//	limitmin=12.0f*limitminfactor;
//	limitmax=12.0f*limitmaxfactor;
//    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_12V_N);
//	rt_thread_delay(MSec(50));
//	MBST_POWER_TEST(10.0f);
//    BC_Debug();

	/*+3V��Դ�Լ�*/
	snprintf(itemname,sizeof(itemname),"+3V �Լ�");
	limitmin=3.0f*limitminfactor;
	limitmax=3.0f*limitmaxfactor;
	r1=510.0f;//��ѹ����1 (KOhm)	: �ӵ�Դ��
	r2=510.0f;//��ѹ����2 (KOhm)	����GND��
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_3V);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

	/*+3.3V��Դ�Լ�*/
	snprintf(itemname,sizeof(itemname),"+3.3V �Լ�");
	limitmin=3.3f*limitminfactor;
	limitmax=3.3f*limitmaxfactor;
	r1=510.0f;//��ѹ����1 (KOhm)	: �ӵ�Դ��
	r2=510.0f;//��ѹ����2 (KOhm)	����GND��
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_3V3);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

	/*+7V��Դ�Լ�*/
	snprintf(itemname,sizeof(itemname),"+7V �Լ�");
	limitmin=7*limitminfactor;
	limitmax=7*limitmaxfactor;
	r1=510.0f;//��ѹ����1 (KOhm)	: �ӵ�Դ��
	r2=100.0f;//��ѹ����2 (KOhm)	����GND��
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_7V_P);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

	/*+5V��Դ�Լ�*/
	snprintf(itemname,sizeof(itemname),"+5V �Լ�");
	limitmin=5*limitminfactor;
	limitmax=5*limitmaxfactor;
	r1=510.0f;//��ѹ����1 (KOhm)	: �ӵ�Դ��
	r2=100.0f;//��ѹ����2 (KOhm)	����GND��
    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_POWER_SELFTEST_5V_P);
	rt_thread_delay(MSec(50));
	MBST_POWER_TEST(((r1+r2)/r2));
    BC_Debug();

    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_CLOSE);
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_CLOSE);
	rt_kprintf("�����Դ�Լ����\n");
}

/*��������Լ죺��������һ��1Kohm�ĵ���*/
void MBST_Res(void)
{
	float limitmin=0.0,limitmax=0.0,testvalue=0.0,adcvalue=0; //������Сֵ���������ֵ������ֵ
	char itemname[32]={0};
	
	snprintf(itemname,sizeof(itemname),"����1Kohm�����Լ�");

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
		rt_kprintf("error:%s����, %s\n",itemname,BC_CONTINE_TIPS);			
		BC_PauseCmd(BC_PAUSE);												
	}

    VOCOM_SelectInputSignal(VOCOM_INPUT_SIGNAL_CLOSE);
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_CLOSE);
	RES_SelectRange(RES_RANGE_CLOSE);
}

void MBST_ADCCal(void)
{
    extern float ADC3GetRealVolt(void);

    float limitmin=0.0,limitmax=0.0,testvalue=0.0; //������Сֵ���������ֵ������ֵ
	char itemname[32]="ADC У׼";
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
		rt_kprintf("error:%s����, %s\n",itemname,BC_CONTINE_TIPS);			
		BC_PauseCmd(BC_PAUSE);												
	}
    ADC1_SelectInputSignal(ADC1_INPUT_SIGNAL_CLOSE);
    
}


