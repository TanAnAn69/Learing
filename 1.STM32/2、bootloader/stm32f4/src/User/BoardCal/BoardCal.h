#ifndef __BOARD_CAL_H
#define __BOARD_CAL_H
#include "stdint.h"
/*�Ƿ�У���־λ*/
#define BC_CAL 1 //��ҪУ��
#define BC_NCAL 0 //����ҪУ��

/*�Ƿ���Ҫ�Լ�ı�־λ*/
#define BC_SELF_TEST   1 //��Ҫ�Լ�
#define BC_NSELF_TEST  0 //����Ҫ�Լ�

/*��ͣ��־λ*/
#define BC_CONTINE 0
#define BC_PAUSE 1
#define BC_CONTINE_TIPS "����������BC_PauseCmd(0)���߰���������"


extern void MBST_Power(void);

extern void MBST_Res(void);
extern void MBST_ADCCal(void);


#define BC_Debug() BC_DebugEx(__FILE__,__LINE__)
extern void BC_PauseCmd(uint8_t iCmd);
void BC_DebugEx(const char* iFile,uint16_t iLine);

extern void SBC_GetRes4WCalFactor(float *k,float *b);
extern void SBC_GetResCalFactor(float *k,float *b);


#endif
