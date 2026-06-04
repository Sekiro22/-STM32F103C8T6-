#include "stm32f10x.h"                  // Device header
#include "u8g2.h"
#include "Timer.h"
#include <stdio.h>

extern u8g2_t u8g2;

uint16_t Timer_Count;
uint8_t Timer_Flag = 0;
//uint32_t Test;
char Timer[10];
//char Test_Time[10];

TIMER_Typedef TIMER_Structure = {0, 0, 0};


/**
  * @brief  初始化 TIM2 为 10ms 周期更新中断，用作秒表的百分之一秒计时基准，但初始化后不立即启动计数。
  * @param  无输入参数；函数固定配置 TIM2 和 TIM2_IRQn，不接收外部配置结构体。
  * @return 无返回值。
  */
void Timer_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_InternalClockConfig(TIM2);
	
	//时基单元初始化
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 6400 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	//清除标志位
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	
	//打开定时器中断
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	//NVIC分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	//NVIC配置
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  TIM2 更新中断服务函数，每次中断累加百分之一秒计数，并在达到 100 个 tick 后进位到秒，秒达到 60 后进位到分钟。
  * @param  无输入参数；中断入口由 NVIC 调用，函数通过全局 TIMER_Structure 输出计时结果。
  * @return 无返回值。
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIMER_Structure.Ms ++;
		if (TIMER_Structure.Ms == 100)
		{
			TIMER_Structure.Ms = 0;
			TIMER_Structure.Sec ++;
			if (TIMER_Structure.Sec == 60)
			{
				TIMER_Structure.Min ++;
			}
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

/**
  * @brief  绘制秒表页面，将 TIMER_Structure 中的分钟、秒和百分之一秒格式化为 MM:SS:CC 并居中显示。
  * @param  无输入参数；函数通过全局 TIMER_Structure、Timer_Flag 和 u8g2 访问计时状态与显示缓冲区。
  * @return 无返回值。
  */
void Timer_Display(void)
{
	if (Timer_Flag == 0)
	{
		Timer_Flag = 1;
	}
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_logisoso24_tn);
	sprintf(Timer, "%d%d:%d%d:%d%d", TIMER_Structure.Min/10, TIMER_Structure.Min%10,
										TIMER_Structure.Sec/10, TIMER_Structure.Sec%10,
										TIMER_Structure.Ms/10, TIMER_Structure.Ms%10);
	u8g2_DrawStr(&u8g2, 64 - u8g2_GetStrWidth(&u8g2, "00:00:00")/2, 32 + 6, Timer);
	
//	Test = SystemCoreClock;
//	sprintf(Test_Time, "%d", Test);
//	u8g2_DrawStr(&u8g2, 0, 64, Test_Time);
	u8g2_SendBuffer(&u8g2);
}
