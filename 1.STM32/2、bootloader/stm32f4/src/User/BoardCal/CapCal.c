/*
 * ���Դʺ��壺
 * SBC_:switch board calibration ��swithc��У׼��
 * SBST_:switch board selftest ��swithc���Լ죩
 */
#include "stdtest_common.h"
extern uint8_t CAPCR_SelectCalRes(eCapCRCalRes iCapCRCalRes);
extern uint16_t BC_EepromAddr;

#define EPA_B_CAP_CAL (BC_EepromAddr+200)


/*�����Ϻ�У׼���ϵ������˵ĵ�ѹ*/ 
typedef struct
{
	float V1;//������У׼����1���˵�ѹ
    float V2;//������У׼����2���˵�ѹ
}sCapCalVolt;

/*����У׼ϵ��*/ 
typedef struct
{
	uint8_t initflag;
    float R1;
    float R2;
    float V1; 
	float V2;
}sCapCalFactor;

/*����У��ʱ��Ҫ�õ��������Ϣ��ȫ����nFΪ��λ*/
typedef struct
{
	const char *RangeName;//��λ����
    uint16_t Range; //��ҪУ׼�ĵ�λ

    const char *R1Name; //���壺����1����
    float Res1;     //���壺����1����ֵ
    
    const char *R2Name; //���壺����2����    
    float Res2;      //���壺����2����ֵ

    float V1Min;//���壺����1���˵�ѹ��Сֵ
    float V1Max;//���壺����1���˵�ѹ���ֵ
    float V2Min;//���壺����2���˵�ѹ��Сֵ
    float V2Max;//���壺����2���˵�ѹ��� ֵ
	uint16_t flag;    //�Ƿ���ҪУ��ı�־λ
}sCapCalInfo;

/*�����Լ�ʱ��Ҫ�õ��������Ϣ��ȫ����nFΪ��λ*/
typedef struct
{
    const char *Name; //��������
    uint16_t Range; //���Դ˵��ݵĵ�λ
	uint16_t HPin; //���ݵĸ�����
	uint16_t LPin; //���ݵĵ�����
    float Cap;     //���ݵ���ֵ
	uint16_t flag; //�Ƿ���Ҫ�Լ�
}sCapSelfTestInfo;

/*��������У׼ʱ�ĵ�����Ϣ*/
static uint8_t CapCalFlag=0; //resУ��ϵ���Ƿ���ȷ  0:CapУ��ϵ������  1:resУ��ϵ����ȷ
static uint8_t CapCalInitFlag=0; //resУ��ϵ���Ƿ��ʼ����0��ϵ��δ��ʼ��  1��ϵ���Ѿ���ʼ��
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

/*���������Լ�ĵ�����Ϣ*/
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

/*��ȡȡУ��ϵ��*/
void SBC_GetCapCalFactor(float *iR1,float *iR2,float *iV1,float *iV2)
{
    float r1,r2;//У׼������������
    float v1,v2;//У׼�������������Ӧ�ĵ�ѹ
    
    FWAssertError(CapCalInitFlag,"Cap У��ϵ��δ��ʼ��!\n");
    FWAssertError(CapCurRange<CAP_RANGE_NUM,"Range error!\n");
    FWAssertError(CapCalFlag,"Cap У��ϵ�����ڴ���������У��!\n");
    FWAssertError(CapCalInfo[CapCurRange].flag==BC_CAL,"�벻Ҫʹ��δУ��ĵ�λ = %s\n",CapCalInfo[CapCurRange].RangeName);
    
    if(CapCalFactor[CapCurRange].initflag==0)
    {
        v1=CapCalVolt[CapCurRange].V1;
        v2=CapCalVolt[CapCurRange].V2;
        
        /*����ϵ��*/
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

/*CapУ��ϵ����ʼ��*/
void SBC_CapCalInit(void)
{
    uint8_t i;
    float v1,v2,calv1,calv2;
    
    PRINT_INIT_START();
    CapCalFlag=1;
    for(i=0;i<CAP_RANGE_NUM;i++)
    {
        if(CapCalInfo[i].flag==BC_NCAL) continue;
        
        /*��eeprom�ж�ȡ�������˵�У���ѹ*/
        calv1=EEPROM_Rdf(EPA_B_CAP_CAL+i*8);
        calv2=EEPROM_Rdf(EPA_B_CAP_CAL+4+i*8);
        
        /*��ӡϵ����Ϣ*/
        rt_kprintf("Cap cal:%s V1=.3%f[%.3f,%.3f] add=%d , V2=%.3f[%.3f,%.3f] add=%d\n",
                    CapCalInfo[i].RangeName,
                    v1,CapCalInfo[i].V1Min ,CapCalInfo[i].V1Max,EPA_B_CAP_CAL+i*8,   
                    v2,CapCalInfo[i].V2Min,CapCalInfo[i].V2Max,EPA_B_CAP_CAL+4+i*8);
        /*�жϵ�ѹ�Ƿ���������*/
        if(v1>=CapCalInfo[i].V1Min && v1<=CapCalInfo[i].V1Max && v2>=CapCalInfo[i].V2Min && v2<=CapCalInfo[i].V2Max)
        {
            CapCalVolt[i].V1=calv1;
            CapCalVolt[i].V2=calv2;
        }
        else
        {
            CapCalFlag=0;//У��ϵ������
            rt_kprintf("--------��λ%s��У��ϵ������----------\n\n",CapCalInfo[i].RangeName);
        }
    }
    CapCalInitFlag=1;
    PRINT_INIT_OK();
}

#include "stdlib_gpio.h"
/*����У׼*/
void SBC_CapCal(void)
{
    uint8_t i,j=0;
    uint8_t result;
    uint16_t delay=300;
    float v1=0.0,v2=0.0;//У׼���ϵ������˵ĵ�ѹ
    
    rt_kprintf("\n\n-----------------------------------   ����У��   ----------------------------------\n\n");
    for(i=0;i<CAP_RANGE_NUM;i++)
    {
        CapCalFactor[i].initflag=0;
        result=1;
        
        if(CapCalInfo[i].flag==BC_NCAL) continue;

        /*ѡ����Ե�λ*/
        CAPCR_SelectRange((eCapRange)CapCalInfo[i].Range);
        CURPN_SelectOutputSignal(CURPN_OUTPUT_SIGNAL_CLOSE);
        
        /*�������1����*/
        if(CapCRCurRangeRes==CAP_CR_RANGE_RES1) CAPCR_SelectCalRes(CAP_CR_CAL_RES1_1);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES2) CAPCR_SelectCalRes(CAP_CR_CAL_RES2_1);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES3) CAPCR_SelectCalRes(CAP_CR_CAL_RES3_1);
        
        CAPCR_NonloadInputDisable();
        rt_thread_delay(MSec(delay));
        v1=ADC3_GetAvgVolt(32);
        BC_Debug();
        CAPCR_NonloadInputEnable();
        
        /*�������2����*/
        if(CapCRCurRangeRes==CAP_CR_RANGE_RES1) CAPCR_SelectCalRes(CAP_CR_CAL_RES1_2);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES2) CAPCR_SelectCalRes(CAP_CR_CAL_RES2_2);
        else if(CapCRCurRangeRes==CAP_CR_RANGE_RES3) CAPCR_SelectCalRes(CAP_CR_CAL_RES3_2);
        
        CAPCR_NonloadInputDisable();
        rt_thread_delay(MSec(delay));
        v2=ADC3_GetAvgVolt(32);
        BC_Debug();
        CAPCR_NonloadInputEnable();

        /*��ӡ�����Ϣ*/
        rt_kprintf("Range=%s , ",CapCalInfo[i].RangeName); //��λ����
        rt_kprintf("%s(%.0f Ohm) volt=%.3f[%.3f,%.3f] , ",CapCalInfo[i].R1Name,CapCalInfo[i].Res1,v1,CapCalInfo[i].V1Min,CapCalInfo[i].V1Max); //����1��Ϣ
        rt_kprintf("%s(%.0f Ohm) volt=%.3f[%.3f,%.3f]\n",CapCalInfo[i].R2Name,CapCalInfo[i].Res2,v2,CapCalInfo[i].V2Min,CapCalInfo[i].V2Max); //����2��Ϣ
        
        /*�жϵ�ѹֵ�Ƿ��ڷ�Χ��*/
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
        
        /*�رմ˵�λ*/
        CAPCR_SelectRange(CAP_RANGE_CLOSE);
    }
    rt_kprintf("-----------------------------------   ����У�����   ----------------------------------\n\n");
}

/*���ߵ����Լ�*/
void SBC_CapSelfTest(void)
{
    uint8_t i;
    float testvalue=0.0,limitmin=0.0,limitmax=0.0;
   
    rt_kprintf("\n\n-----------------------------------   �����Լ�   ----------------------------------\n\n");
    for(i=0;i<CAP_SELF_TEST_NUM;i++)
    {
        if(CapSelfTestInfo[i].flag==BC_NSELF_TEST) continue;
        
        /*��������*/
        limitmin=CapSelfTestInfo[i].Cap*0.9f;
        limitmax=CapSelfTestInfo[i].Cap*1.1f;
        
        /*ѡ����Ե�λ*/
        CAPCR_SelectRange((eCapRange)CapSelfTestInfo[i].Range);
        rt_thread_delay(MSec(20)); //���ڲ�ͬ��λ���˴���ʱʱ�䲻ͬ�����Գ�����ֵ��ͬ
        
        /*���ݲ���*/
        Port_Connect(CapSelfTestInfo[i].HPin-1,VOPCUR_CON);
        Port_Connect(CapSelfTestInfo[i].LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(20)); //���ڲ�ͬ����ֵ���˴���ʱʱ�䲻ͬ�����Գ�����ֵ��ͬ
        CAP_Measure(nF,50,&testvalue);
        
        /*��ӡ�����Ϣ*/
        rt_kprintf("%s(%f nF) Cap=%f[%f,%f] ",CapSelfTestInfo[i].Name,CapSelfTestInfo[i].Cap,testvalue,limitmin,limitmax);
        rt_kprintf("ADC=%fV CalV1=%f CalV2=%f\n",ADC3_GetAvgVolt(32),CapCalVolt[CapSelfTestInfo[i].Range].V1,CapCalVolt[CapSelfTestInfo[i].Range].V2);
        
        /*�ж�����*/
        if(testvalue<limitmin || testvalue>limitmax) 
        {
            rt_kprintf("Cap is error . %s\n",BC_CONTINE_TIPS);
            BC_PauseCmd(BC_PAUSE);
        }
        
        Port_Disconnect(CapSelfTestInfo[i].HPin-1,VOPCUR_CON);
        Port_Disconnect(CapSelfTestInfo[i].LPin-1,COMNCUR_CON);
        rt_thread_delay(MSec(10));
        
        /*�رմ˵�λ*/
        CAPCR_SelectRange(CAP_RANGE_CLOSE);
    }
    
    rt_kprintf("-----------------------------------   �����Լ����   ----------------------------------\n\n");    
}



