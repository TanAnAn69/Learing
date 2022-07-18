/*************************************************************************
 * File     : stdlib_gpio.h
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-10      aaron           first version
 ************************************************************************/
#ifndef __STDLIB_GPIO_H
#define __STDLIB_GPIO_H
/*** Including ***/
#include "stdint.h"
/*****************************************************/
/*** MACRO DEFINITION AREA ***/
/*****************************************************/
/*** Õ®”√IOµÿ÷∑ ***/
#define PA0     0xC000
#define PA1     0xC001
#define PA2     0xC002
#define PA3     0xC003
#define PA4     0xC004
#define PA5     0xC005
#define PA6     0xC006
#define PA7     0xC007
#define PA8     0xC008
#define PA9     0xC009
#define PA10    0xC00A
#define PA11    0xC00B
#define PA12    0xC00C
#define PA13    0xC00D
#define PA14    0xC00E
#define PA15    0xC00F

#define PB0     0xC100
#define PB1     0xC101
#define PB2     0xC102
#define PB3     0xC103
#define PB4     0xC104
#define PB5     0xC105
#define PB6     0xC106
#define PB7     0xC107
#define PB8     0xC108
#define PB9     0xC109
#define PB10    0xC10A
#define PB11    0xC10B
#define PB12    0xC10C
#define PB13    0xC10D
#define PB14    0xC10E
#define PB15    0xC10F

#define PC0     0xC200
#define PC1     0xC201
#define PC2     0xC202
#define PC3     0xC203
#define PC4     0xC204
#define PC5     0xC205
#define PC6     0xC206
#define PC7     0xC207
#define PC8     0xC208
#define PC9     0xC209
#define PC10    0xC20A
#define PC11    0xC20B
#define PC12    0xC20C
#define PC13    0xC20D
#define PC14    0xC20E
#define PC15    0xC20F

#define PD0     0xC300
#define PD1     0xC301
#define PD2     0xC302
#define PD3     0xC303
#define PD4     0xC304
#define PD5     0xC305
#define PD6     0xC306
#define PD7     0xC307
#define PD8     0xC308
#define PD9     0xC309
#define PD10    0xC30A
#define PD11    0xC30B
#define PD12    0xC30C
#define PD13    0xC30D
#define PD14    0xC30E
#define PD15    0xC30F

#define PE0     0xC400
#define PE1     0xC401
#define PE2     0xC402
#define PE3     0xC403
#define PE4     0xC404
#define PE5     0xC405
#define PE6     0xC406
#define PE7     0xC407
#define PE8     0xC408
#define PE9     0xC409
#define PE10    0xC40A
#define PE11    0xC40B
#define PE12    0xC40C
#define PE13    0xC40D
#define PE14    0xC40E
#define PE15    0xC40F

#define PF0     0xC500
#define PF1     0xC501
#define PF2     0xC502
#define PF3     0xC503
#define PF4     0xC504
#define PF5     0xC505
#define PF6     0xC506
#define PF7     0xC507
#define PF8     0xC508
#define PF9     0xC509
#define PF10    0xC50A
#define PF11    0xC50B
#define PF12    0xC50C
#define PF13    0xC50D
#define PF14    0xC50E
#define PF15    0xC50F

#define PG0     0xC600
#define PG1     0xC601
#define PG2     0xC602
#define PG3     0xC603
#define PG4     0xC604
#define PG5     0xC605
#define PG6     0xC606
#define PG7     0xC607
#define PG8     0xC608
#define PG9     0xC609
#define PG10    0xC60A
#define PG11    0xC60B
#define PG12    0xC60C
#define PG13    0xC60D
#define PG14    0xC60E
#define PG15    0xC60F

#define PH0     0xC700
#define PH1     0xC701
#define PH2     0xC702
#define PH3     0xC703
#define PH4     0xC704
#define PH5     0xC705
#define PH6     0xC706
#define PH7     0xC707
#define PH8     0xC708
#define PH9     0xC709
#define PH10    0xC70A
#define PH11    0xC70B
#define PH12    0xC70C
#define PH13    0xC70D
#define PH14    0xC70E
#define PH15    0xC70F

#define PI0     0xC800
#define PI1     0xC801
#define PI2     0xC802
#define PI3     0xC803
#define PI4     0xC804
#define PI5     0xC805
#define PI6     0xC806
#define PI7     0xC807
#define PI8     0xC808
#define PI9     0xC809
#define PI10    0xC80A
#define PI11    0xC80B
#define PI12    0xC80C
#define PI13    0xC80D
#define PI14    0xC80E
#define PI15    0xC80F




/*****************************************************/
/*** DATA TYPE AREA ***/
/*****************************************************/

/*****************************************************/
/*** VARIABLE DECLARATION AREA ***/
/*****************************************************/

/*****************************************************/
/*** FUNCTION DECLARATION AREA ***/
/*****************************************************/
void SubSetIO(uint16_t addr, uint8_t state);
uint8_t SubReadIO(uint16_t addr);
void SetIO(const char *name, uint8_t state);
uint8_t ReadIO(const char *name);

extern void SubGPIOInitOut(uint16_t iAddr);
extern void SubGPIOWriteBit(uint16_t iAddr,uint8_t iState);

extern void SubGPIOInitIn(uint16_t iAddr);

uint8_t ReadIORepeat(uint16_t addrIO, uint8_t iState, uint8_t times, uint8_t delay);
void WaitIOTips(uint16_t addrIO, uint8_t value, int32_t iTimeOut, const char *iTips);

uint8_t ReadMultIORepeat(uint8_t iMultState, uint8_t times, uint8_t delay, uint8_t numIO, ...);
uint8_t ReadMultIORepeatS(uint8_t iMultState, uint8_t times, uint8_t delay, uint8_t numIO, uint16_t *pIOAddr);

void WaitMultIOTips(uint8_t stateIO, int32_t iTimeOut, const char *iTips, uint8_t numIO, ...);

#endif  // __STDLIB_GPIO_H
