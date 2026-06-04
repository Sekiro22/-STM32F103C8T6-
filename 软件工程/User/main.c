#include "main.h"

u8g2_t u8g2;

/**
  * @brief  系统入口函数，按固定顺序初始化蜂鸣器、LED、按键、RTC、TIM2 秒表计时器和 OLED，然后在主循环中持续执行菜单调度与页面刷新。
  * @param  无输入参数；启动参数由 C 运行环境和单片机复位流程提供，本函数不接收外部指针或配置对象。
  * @return 正常情况下不返回；若异常跳出主循环，返回值按 C 语言入口约定为整型状态码，但当前实现没有成功或失败分支。
  */
int main(void)
{
	Buzzer_Init();
	LED_Init();
	Key_Init();
	MyRTC_Init();
	Timer_Init();
	u8g2_Init(&u8g2);
	while (1)
	{
		Menu_Key_Set();
	}
}
