#include "stm32f10x.h"

/**
  * @brief  使用 SysTick 进行阻塞式微秒延时，延时期间 CPU 忙等直到计数完成。
  * @param  xus 输入参数；表示需要延时的微秒数，不是指针不允许为空，受 SysTick 24 位重装载寄存器限制，过大值会导致装载值溢出。
  * @return 无返回值。
  */
void Delay_us(uint32_t xus)
{
	SysTick->LOAD = 72 * xus;
	SysTick->VAL = 0x00;
	SysTick->CTRL = 0x00000005;
	while(!(SysTick->CTRL & 0x00010000));
	SysTick->CTRL = 0x00000004;
}

/**
  * @brief  通过循环调用 Delay_us(1000) 实现阻塞式毫秒延时。
  * @param  xms 输入参数；表示需要延时的毫秒数，不是指针不允许为空，取值越大阻塞时间越长。
  * @return 无返回值。
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}

/**
  * @brief  通过循环调用 Delay_ms(1000) 实现阻塞式秒级延时。
  * @param  xs 输入参数；表示需要延时的秒数，不是指针不允许为空，取值越大阻塞时间越长。
  * @return 无返回值。
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
}
