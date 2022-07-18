/*************************************************************************
 * File     : stdlib_bitoperator.h
 * Author   : platform team
 * COPYRIGHT (C) 2017, YANMADE NewPlatform Team
 *
 * Description:
 *
 * Change Logs:
 * Date            Author          Notes
 * 2017-07-10      aaron           first version
 ************************************************************************/
#ifndef __STDLIB_BITOPERATOR_H
#define __STDLIB_BITOPERATOR_H
/*** Including ***/

/*****************************************************/
/*** MACRO DEFINITION AREA ***/
/*****************************************************/
#define BitSet(data, bitnum)        ((data) = (data) | (1 << (bitnum)))
#define BitClear(data, bitnum)      ((data) = (data) & (~(1 << (bitnum))))
#define BitGet(data, bitnum)        (((data) & (1 << (bitnum))) >> (bitnum))

/*****************************************************/
/*** DATA TYPE AREA ***/
/*****************************************************/

/*****************************************************/
/*** VARIABLE DECLARATION AREA ***/
/*****************************************************/

/*****************************************************/
/*** FUNCTION DECLARATION AREA ***/
/*****************************************************/

#endif  // __STDLIB_BITOPERATOR_H
