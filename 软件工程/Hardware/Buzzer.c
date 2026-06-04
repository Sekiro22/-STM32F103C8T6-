#include "stm32f10x.h"                  // Device header

/**
  * @brief  初始化蜂鸣器 PWM 输出，将 PB8 配置为 TIM4_CH3 复用推挽输出，并配置约 330Hz 的 PWM 基准但默认占空比为 0。
  * @param  无输入参数；函数内部固定使用 GPIOB、PB8、TIM4 和通道 3，不接收可为空配置。
  * @return 无返回值。
  */
void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 1000000/330 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 64 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
}

/**
  * @brief  根据状态参数打开或关闭蜂鸣器 PWM 输出，打开时设置 TIM4_CH3 比较值，关闭时清零比较值并停止 TIM4。
  * @param  State 输入参数；取值为 1 时打开蜂鸣器，取值为 0 时关闭蜂鸣器，其他非 1 取值按关闭处理；参数不是指针，不存在为空情况。
  * @return 无返回值。
  */
void Buzzer_State(uint8_t State)
{
	if (State == 1)
	{
		TIM_SetCompare3(TIM4, 330);
		TIM_Cmd(TIM4, ENABLE);
	}
	else
	{
		TIM_SetCompare3(TIM4, 0);
		TIM_Cmd(TIM4, DISABLE);
	}
}
