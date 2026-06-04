#include "main.h"

u8g2_t u8g2;

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
