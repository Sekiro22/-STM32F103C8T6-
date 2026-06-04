#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Delay.h"
#include "Timer.h"

volatile uint16_t Select_flag;
extern uint8_t Timer_Flag;
extern TIMER_Typedef TIMER_Structure;
extern int MainMenu_Str_y;
uint8_t Timer_State;

/**
  * @brief  初始化 PA0、PA1、PA2、PA8、PA9 为上拉输入，并配置为下降沿 EXTI 中断，用于产生菜单和秒表控制按键事件。
  * @param  无输入参数；函数固定配置 GPIOA、AFIO、EXTI0/1/2/8/9 和对应 NVIC 通道，不接收外部配置对象。
  * @return 无返回值。
  */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource9);
	
	EXTI_DeInit();
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line8 | EXTI_Line9; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  PA0 确认键中断服务函数，普通页面设置 Select_flag=3，秒表页面切换 TIM2 启停状态。
  * @param  无输入参数；中断入口由 NVIC 调用，按键状态通过 GPIOA Pin0 读取，输出为全局 Select_flag 或 Timer_State。
  * @return 无返回值。
  */
void EXTI0_IRQHandler(void)
{
 	if (EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		Delay_ms(20);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
		{
			MainMenu_Str_y = 74;
			if(Timer_Flag == 1)
			{
				if (Timer_State == 1)
				{
					TIM_Cmd(TIM2, DISABLE);
					Timer_State = 0;
				}
				else
				{
					TIM_Cmd(TIM2, ENABLE);
					Timer_State = 1;
				}
			}
			else
			{
				Select_flag = 3;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

/**
  * @brief  PA1 上一个键中断服务函数，普通页面设置 Select_flag=1，秒表页面清零分钟、秒和百分之一秒计数。
  * @param  无输入参数；中断入口由 NVIC 调用，按键状态通过 GPIOA Pin1 读取，输出为全局 Select_flag 或 TIMER_Structure。
  * @return 无返回值。
  */
void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) == SET)
	{
		Delay_ms(20);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0)
		{
			if (Timer_Flag == 1)
			{
				TIMER_Structure.Min = 0;
				TIMER_Structure.Ms = 0;
				TIMER_Structure.Sec = 0;
			}
			else
			{
				Select_flag = 1;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

/**
  * @brief  PA2 下一个键中断服务函数，消抖确认后设置 Select_flag=2。
  * @param  无输入参数；中断入口由 NVIC 调用，按键状态通过 GPIOA Pin2 读取，输出为全局 Select_flag。
  * @return 无返回值。
  */
void EXTI2_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		Delay_ms(20);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0)
		{
			Select_flag = 2;
		}
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

/**
  * @brief  PA8/PA9 组合中断服务函数，PA8 触发返回事件 Select_flag=4，PA9 触发主页事件 Select_flag=5，并在离开秒表时清除 Timer_Flag。
  * @param  无输入参数；中断入口由 NVIC 调用，分别读取 GPIOA Pin8 和 Pin9，输出为全局 Select_flag 与 Timer_Flag。
  * @return 无返回值。
  */
void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line8) == SET)
	{
		Delay_ms(20);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0)
		{
			MainMenu_Str_y = 74;
			if (Timer_Flag == 1)
			{
				Timer_Flag = 0;
			}
			Select_flag = 4;
		}
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	if (EXTI_GetITStatus(EXTI_Line9) == SET)
	{
		Delay_ms(20);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == 0)
		{
			if (Timer_Flag == 1)
			{
				Timer_Flag = 0;
			}
			Select_flag = 5;
		}
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}
