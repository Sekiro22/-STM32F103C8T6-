#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Delay.h"
#include "Timer.h"

volatile uint16_t Select_flag;
extern uint8_t Timer_Flag;
extern TIMER_Typedef TIMER_Structure;
extern int MainMenu_Str_y;
uint8_t Timer_State;

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
