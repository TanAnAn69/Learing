#ifndef __BOARD_CAL_H
#define __BOARD_CAL_H
#include "stdint.h"
/*是否校验标志位*/
#define BC_CAL 1 //需要校验
#define BC_NCAL 0 //不需要校验

/*是否需要自检的标志位*/
#define BC_SELF_TEST   1 //需要自检
#define BC_NSELF_TEST  0 //不需要自检

/*暂停标志位*/
#define BC_CONTINE 0
#define BC_PAUSE 1
#define BC_CONTINE_TIPS "请输入命令BC_PauseCmd(0)或者按按键继续"


extern void MBST_Power(void);

extern void MBST_Res(void);
extern void MBST_ADCCal(void);


#define BC_Debug() BC_DebugEx(__FILE__,__LINE__)
extern void BC_PauseCmd(uint8_t iCmd);
void BC_DebugEx(const char* iFile,uint16_t iLine);

extern void SBC_GetRes4WCalFactor(float *k,float *b);
extern void SBC_GetResCalFactor(float *k,float *b);


#endif
