#include <stdio.h>
#include "u8g2.h"
#include "MyRTC.h"
#include "OLED.h"

extern uint8_t Power_Flag;
extern u8g2_t u8g2;

/**
  * @brief  读取 RTC 当前时间并绘制时钟主界面，包括日期、星期、时分和秒；若此前进入过低功耗关闭 OLED，则先恢复 OLED 显示。
  * @param  无输入参数；函数通过全局 RTC_SetTime、Power_Flag 和 u8g2 访问 RTC 数据与显示缓冲区，不接收可为空指针。
  * @return 无返回值。
  */
void Clock_Display(void)
{
	extern RTC_Time RTC_SetTime;
	char Sec[1];
	char Time[4];
	char Date[10];
	if (Power_Flag == 0)
	{
		u8g2_SetPowerSave(&u8g2, 0);
		Power_Flag = 1;
	}
	
	MyRTC_ReadTime();
	
	u8g2_ClearBuffer(&u8g2);
	
	u8g2_SetFont(&u8g2, u8g2_font_t0_11b_tr);
//	sprintf(Date, "%d%d%d%d-%d%d-%d%d", RTC_SetTime.Year/1000, RTC_SetTime.Year%1000/100, RTC_SetTime.Year%100/10, RTC_SetTime.Year%10,
//										RTC_SetTime.Mon/10, RTC_SetTime.Mon%10, RTC_SetTime.Day/10, RTC_SetTime.Day%10);
	sprintf(Date, "%d-%d-%d", RTC_SetTime.Year, RTC_SetTime.Mon, RTC_SetTime.Day);
	u8g2_DrawStr(&u8g2, 0, 8, Date);
	switch (RTC_SetTime.Week)
    {
    	case 1: 
			u8g2_DrawStr(&u8g2, 110, 8, "Mon");
    		break;
    	case 2:
			u8g2_DrawStr(&u8g2, 110, 8, "Tue");
    		break;
		case 3:
			u8g2_DrawStr(&u8g2, 110, 8, "Wed");
    		break;
		case 4:
			u8g2_DrawStr(&u8g2, 98, 8, "Thurs");
    		break;
		case 5:
			u8g2_DrawStr(&u8g2, 110, 8, "Fri");
    		break;
		case 6:
			u8g2_DrawStr(&u8g2, 110, 8, "Sat");
    		break;
		case 7:
			u8g2_DrawStr(&u8g2, 110, 8, "Sun");
    		break;
    	default:
    		break;
    }
	
	u8g2_SetFont(&u8g2, u8g2_font_freedoomr25_tn);
	sprintf(Time, "%d%d:%d%d", RTC_SetTime.Hour/10, RTC_SetTime.Hour%10, RTC_SetTime.Min/10, RTC_SetTime.Min%10);
	u8g2_DrawStr(&u8g2, 18, 45, Time);
	
	u8g2_SetFont(&u8g2, u8g2_font_freedoomr10_mu );
	sprintf(Sec, "%d%d", RTC_SetTime.Sec/10, RTC_SetTime.Sec%10);
	u8g2_DrawStr(&u8g2, 106, 45, Sec);
	
	u8g2_SendBuffer(&u8g2);
}
