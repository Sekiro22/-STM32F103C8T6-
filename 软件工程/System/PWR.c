#include "stm32f10x.h"                  // Device header
#include "u8g2.h"

uint8_t Power_Flag = 1;
extern u8g2_t u8g2;

/**
  * @brief  关闭 OLED 显示并进入 STM32 STOP 低功耗模式，唤醒后调用 SystemInit 恢复系统时钟配置。
  * @param  无输入参数；函数通过全局 u8g2 和 Power_Flag 管理显示与电源状态，不接收可为空指针。
  * @return 无返回值。
  */
void Power_Disable(void)
{
	if (Power_Flag == 1)
	{
		Power_Flag = 0;
	}
	u8g2_ClearBuffer(&u8g2);
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 1);
	PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
	SystemInit();
}
