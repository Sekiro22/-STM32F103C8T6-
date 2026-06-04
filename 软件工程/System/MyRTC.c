#include "stm32f10x.h"                  // Device header
#include <time.h>
#include "MyRTC.h"

#define LSI		1
uint16_t RetryCnt = 0;

RTC_Time RTC_SetTime = {2024, 8, 21, 23, 59, 55, 4};

/**
  * @brief  初始化 STM32 内部 RTC 和备份域，首次运行时配置 LSI/LSE 时钟源、预分频器和默认时间，后续运行只恢复同步并保持已有计数值。
  * @param  无输入参数；函数固定访问 RCC、PWR、BKP 和 RTC 外设寄存器，不接收外部配置指针。
  * @return 无返回值。
  */
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

/**
  * @brief  将全局 RTC_SetTime 结构体中的年月日时分秒转换为 RTC 计数器秒值，并写入硬件 RTC。
  * @param  无输入参数；输入数据来自全局 RTC_SetTime，不接收指针；RTC_SetTime 的年月日时分秒应构成有效本地时间。
  * @return 无返回值。
  */
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

/**
  * @brief  从硬件 RTC 计数器读取当前秒值，按 UTC+8 偏移转换为本地时间，并更新全局 RTC_SetTime。
  * @param  无输入参数；函数直接读取 RTC 计数器并写入全局 RTC_SetTime，不接收可为空对象。
  * @return 无返回值。
  */
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
