#include "stm32f10x.h"                  // Device header

/**
  * @brief  初始化板载 LED 使用的 PC13 引脚，将其配置为 50MHz 推挽输出并默认拉低。
  * @param  无输入参数；函数内部固定使用 GPIOC 和 GPIO_Pin_13，不接收可为空的配置指针。
  * @return 无返回值。
  */
void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//开启GPIOA的时钟
															//使用各个外设前必须开启时钟，否则对外设的操作无效
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;					//定义结构体变量
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//GPIO模式，赋值为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				//GPIO引脚，赋值为第0号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//GPIO速度，赋值为50MHz
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//将赋值后的构体变量传递给GPIO_Init函数
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

/**
  * @brief  读取 PC13 当前输出状态并执行翻转，用于切换板载 LED 的亮灭状态。
  * @param  无输入参数；函数内部直接访问 GPIOC 输出寄存器，不依赖外部传入对象。
  * @return 无返回值。
  */
void LED_TurnState(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}else
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
}
