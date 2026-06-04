#include "stm32f10x.h"                  // Device header
#include "u8g2.h"

uint8_t Power_Flag = 1;
extern u8g2_t u8g2;

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
