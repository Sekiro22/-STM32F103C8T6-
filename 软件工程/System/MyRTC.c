#include "stm32f10x.h"                  // Device header
#include <time.h>
#include "MyRTC.h"

#define LSI		1
uint16_t RetryCnt = 0;

RTC_Time RTC_SetTime = {2024, 8, 21, 23, 59, 55, 4};

void MyRTC_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//开启PWR的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);		//开启BKP的时钟
	
	/*备份寄存器访问使能*/
	PWR_BackupAccessCmd(ENABLE);							//使用PWR开启对备份寄存器的访问
	
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)			//通过写入备份寄存器的标志位，判断RTC是否是第一次配置
															//if成立则执行第一次的RTC配置
	{
		#if LSI == 0
		RCC_LSEConfig(RCC_LSE_ON);							//开启LSE时钟
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);	//等待LSE准备就绪
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);				//选择RTCCLK来源为LSE
		RCC_RTCCLKCmd(ENABLE);								//RTCCLK使能
		
		RTC_WaitForSynchro();								//等待同步
		RTC_WaitForLastTask();								//等待上一次操作完成
		
		RTC_SetPrescaler(3000);						//设置RTC预分频器，预分频后的计数频率为1Hz
		RTC_WaitForLastTask();								//等待上一次操作完成
		
		MyRTC_SetTime();									//设置时间，调用此函数，全局数组里时间值刷新到RTC硬件电路
		
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);			//在备份寄存器写入自己规定的标志位，用于判断RTC是不是第一次执行配置
		#else
		RCC_LSICmd(ENABLE);
		RetryCnt = 10000;
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		{if(RetryCnt-- == 0)break;}						//超时退出，防止死机
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);								//RTCCLK使能
		
		RTC_WaitForSynchro();								//等待同步
		RTC_WaitForLastTask();								//等待上一次操作完成
		
		RTC_SetPrescaler(40000 - 1);								//设置RTC预分频器，预分频后的计数频率为1Hz
		RTC_WaitForLastTask();								//等待上一次操作完成
		
		MyRTC_SetTime();									//设置时间，调用此函数，全局数组里时间值刷新到RTC硬件电路
		
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);			//在备份寄存器写入自己规定的标志位，用于判断RTC是不是第一次执行配置
		#endif
	}
	else													//RTC不是第一次配置
	{
		#if LSI == 0
		RTC_WaitForSynchro();								//等待同步
		RTC_WaitForLastTask();								//等待上一次操作完成
		#else
		RCC_LSICmd(ENABLE);
		RetryCnt = 10000;
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		{if(RetryCnt-- == 0)break;}						//超时退出，防止死机
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);								//RTCCLK使能
		
		RTC_WaitForSynchro();								//等待同步
		RTC_WaitForLastTask();								//等待上一次操作完成
		
		RTC_SetPrescaler(40000 - 1);								//设置RTC预分频器，预分频后的计数频率为1Hz
		RTC_WaitForLastTask();								//等待上一次操作完成
		#endif
	}
}

void MyRTC_SetTime(void)
{
	time_t time_cnt;
	struct tm time_data;
	
	time_data.tm_year = RTC_SetTime.Year - 1900;
	time_data.tm_mon = RTC_SetTime.Mon - 1;
	time_data.tm_mday = RTC_SetTime.Day;
	time_data.tm_hour = RTC_SetTime.Hour;
	time_data.tm_min = RTC_SetTime.Min;
	time_data.tm_sec = RTC_SetTime.Sec;
	time_data.tm_wday = RTC_SetTime.Week - 1;
	
	time_cnt = mktime(&time_data) - 8 * 60 * 60;
	
	RTC_SetCounter(time_cnt);
	RTC_WaitForLastTask();
}

void MyRTC_ReadTime(void)
{
	time_t time_cnt;
	struct tm time_data;
	
	time_cnt = RTC_GetCounter() + 8 * 60 * 60;
	
	time_data = *localtime(&time_cnt);
	
	RTC_SetTime.Year = time_data.tm_year + 1900;
	RTC_SetTime.Mon = time_data.tm_mon + 1;
	RTC_SetTime.Day = time_data.tm_mday;
	RTC_SetTime.Hour = time_data.tm_hour;
	RTC_SetTime.Min = time_data.tm_min;
	RTC_SetTime.Sec = time_data.tm_sec;
	RTC_SetTime.Week = time_data.tm_wday + 1;
}
