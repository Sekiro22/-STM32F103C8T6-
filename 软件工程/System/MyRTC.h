#ifndef __MYRTC_H__
#define __MYRTC_H__

#include "stm32f10x.h"                  // Device header

typedef struct MyTime
{
	uint16_t Year;
	uint8_t Mon;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Min;
	uint8_t Sec;
	uint8_t Week;
} RTC_Time;

extern RTC_Time RTC_SetTime;

void MyRTC_SetTime(void);
void MyRTC_Init(void);
void MyRTC_ReadTime(void);

#endif
