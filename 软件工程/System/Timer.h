#ifndef __TIMER_H__
#define __TIMER_H__

#include "stm32f10x.h"                  // Device header

typedef struct
{
	uint8_t Ms;
	uint8_t Sec;
	uint8_t Min;
} TIMER_Typedef;

void Timer_Init(void);
void Timer_Display(void);

#endif
