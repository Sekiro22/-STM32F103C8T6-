#include "MEUN.h"
#include "u8g2.h"
#include "BMP_Lib.h"
#include "Clock.h"
#include "LED.h"
#include "Timer.h"
#include "PWR.h"
#include "Buzzer.h"

extern u8g2_t u8g2;
extern uint8_t Select_flag;

//处理ui界面移动速度 值最好是2的倍数
int speed = 4;
int c_speed = 2;

/**
  * @brief  将当前坐标值按指定速度逐步移动到目标坐标，用于菜单图标、文字、选择框和滚动条的缓动动画。
  * @param  now 输入/输出参数；指向当前坐标值，函数会直接修改其内容，不允许为空，否则会发生非法访问。
  * @param  trag 输入参数；指向目标坐标值，函数只读取其内容，不允许为空，目标值可大于、小于或等于当前值。
  * @param  speed 输入参数；普通移动步长，不是指针不允许为空，建议为正数，值越大动画越快。
  * @param  c_speed 输入参数；接近目标时使用的最小步长，不是指针不允许为空，建议为正数，用于避免尾段移动过慢。
  * @return 返回 1 表示当前值已经等于目标值，返回 0 表示本次调用仍在移动过程中；当前实现没有错误码。
  */
int run_str(int *now,int *trag,const int speed,const int c_speed)
{
	int temp = 0;
	if (*now > *trag)
	{
		temp = ((*now - *trag) > c_speed) ? speed : c_speed;
		*now -= temp;
	}
	else if (*now < *trag)
	{
		temp = ((*trag - *now) > c_speed) ? speed : c_speed;
		*now += temp;
	}
	else 
	{
		return 1;
	}
	return 0;
}

//菜单显示图片参数
int MainMenu_Select = 1;
int MainMenu_Picture_x = -22;
int MainMenu_Picture_x_target = -22;
int MainMenu_MaxNum = 6;//菜单显示最大数量 以0开始计数
//菜单显示字符名字参数
int MainMenu_Str_y = 74;
int MainMenu_Str_x = 64;
int MainMenu_Str_y_target = 60;
//菜单左侧弹出
int MainMenu_Rec_x = -6;
int MainMenu_Rec_x_target = 0;

MainSet_Str MainMenu_Parameter[] = 
{
	{"Setting",7,0},
	{"Clock",5,0},
	{"Timer",5,0},
	{"Calendar",8,0},
	{"Weather",7,0},
	{"Games",5,0},
	{"Power",5,0},
};

Menu_Operate Table[] = {
	{0,0,0,1,0,Clock_Display},
	{1,9,10,2,0,ui_show},
	//1级菜单
	{2,9,10,11,1,String_show},
	{3,9,10,0,1,Clock_Display},	
	{4,9,10,13,1,Timer_Display},	
	{5,9,10,11,1,String_show},
	{6,9,10,11,1,String_show},	
	{7,9,10,12,1,Game_Display},
	{8,9,10,14,1,Power_Disable},
	//坐标操作
	{9,9,9,9,9,Str_Coordinate_Add},
	{10,10,10,10,10,Str_Coordinate_Decrease},	
	 //2级菜单
	{11,11,11,11,11,Str_Operate},
	{12,12,12,12,1,Game_Display},
	{13,13,13,13,1,Timer_Display},
	{14,14,14,14,0,Power_Disable},
};

MainSet_Str_All Str_AllArray[] = 
{
	{
		{{"aetting",7,0},{"alock",5,0},{"aimer",5,0},{"aalendar",8,0},},
	    0,3,4,4,12,12,0,0,0,0,0,14,2,7,2,2,0},
	{0},
	{	{{"cetting",7,0},{"clock",5,0},{"cimer",5,0},{"calendar",8,0},{"ceather",7,0},{"cames",5,0},},
	    0,5,4,4,12,12,0,0,0,0,0,14,7,7,2,2,0},
	{   {{"detting",7,0},{"dlock",5,0},{"cimer",5,0},{"dalendar",8,0},{"eeather",7,0},{"fames",5,0},},
	    0,5,4,4,12,12,0,0,0,0,0,14,7,7,2,2,0},
	{   {{"eetting",7,0},{"elock",5,0},{"cimer",5,0},{"dalendar",8,0},{"eeather",7,0},{"fames",5,0},},
	    0,5,4,4,12,12,0,0,0,0,0,14,7,7,2,2,0},
	{0},
	{0},
};

int func_index = 0;
int func_index_last = 0;
void (*current_operation_index)(void);

/**
  * @brief  绘制主菜单横向图标轮播和当前选中项文字，并更新图标、文字和提示条坐标以形成滑动动画。
  * @param  无输入参数；函数通过全局 MainMenu_* 状态、logo 位图数组和 u8g2 显示对象完成绘制，不接收外部指针。
  * @return 无返回值。
  */
void ui_show(void)
{
	char i = 0;
	u8g2_SetFont(&u8g2, u8g2_font_ImpactBits_tr);//设置菜单字符名字显示字体
	
/*  显示代码段  */	
	
	for (i = 0;i <= MainMenu_MaxNum;i++)//显示菜单图片
	{
		u8g2_DrawXBMP(&u8g2,MainMenu_Picture_x+i*64, 0, 44, 44, logo[i]);
	}
	
	if (MainMenu_Select >= 0 && MainMenu_Select <= MainMenu_MaxNum)//显示菜单的字符名字
	{
		u8g2_DrawStr(&u8g2,MainMenu_Str_x - u8g2_GetStrWidth(&u8g2, MainMenu_Parameter[MainMenu_Select].str)/2,MainMenu_Str_y,MainMenu_Parameter[MainMenu_Select].str);
		//u8g2_GetStrWidth(&u8g2, MainMenu_Parameter[MainMenu_Select].str);
		//(MainMenu_Parameter[MainMenu_Select].num)*6/2;
	}
//	u8g2_DrawBox(&u8g2, MainMenu_Rec_x, 50, 8, 16);//左边弹出矩形

/*  处理坐标代码段  */		
	run_str(&MainMenu_Picture_x,&MainMenu_Picture_x_target,32,c_speed);//对图片坐标进行移动
	run_str(&MainMenu_Str_y,&MainMenu_Str_y_target,speed,c_speed);//对图片字符名称坐标进行移动
	run_str(&MainMenu_Rec_x,&MainMenu_Rec_x_target,speed,c_speed);//弹出动画
}

/**
  * @brief  菜单主调度函数，读取按键中断写入的 Select_flag，根据 Table 状态表更新 func_index，并调用当前页面或动作函数后刷新 OLED 缓冲区。
  * @param  无输入参数；函数通过全局 Select_flag、func_index、Table 和 u8g2 完成状态转换与显示输出。
  * @return 无返回值。
  */
void Menu_Key_Set(void)
{	
	func_index_last = func_index;
	if (Select_flag > 0)
	{
		switch (Select_flag)
		{
			case 1: func_index = Table[func_index].upper;break;
			case 2: func_index = Table[func_index].next;break;
			case 3: func_index = Table[func_index].enter;
					if (func_index == UI_Next_Addr)
					{
						func_index += MainMenu_Select;
					}
					break;
			case 4: func_index = Table[func_index].back;break;
			case 5: func_index = 0;break;
			default: break;
		}
		Select_flag = 0;		
	}
//
	if (func_index == 9 || func_index == 10||func_index == 11)
	{
		current_operation_index = Table[func_index].current_operation;
		(*current_operation_index)();	
	}
	current_operation_index = Table[func_index].current_operation;
	(*current_operation_index)();			
	
	u8g2_SendBuffer(&u8g2);
	u8g2_ClearBuffer(&u8g2);
}

/**
  * @brief  处理菜单“下一个/坐标增加”动作，主菜单中切换到右侧图标，二级菜单中下移选择项并更新选择框和滚动条目标坐标。
  * @param  无输入参数；函数通过全局 func_index_last、MainMenu_Select、MainMenu_* 和 Str_AllArray 修改菜单状态。
  * @return 无返回值。
  */
void Str_Coordinate_Add(void)
{
	//对主界面坐标操作
	if (func_index_last == Y0)//图片向左移动
	{
		MainMenu_Select += 1;
		MainMenu_Picture_x_target -= 64;		
		if (MainMenu_Select >= 0 && MainMenu_Select <= MainMenu_MaxNum)
		{	
			//模拟字符从屏幕下方弹起效果		
			MainMenu_Str_y = 74;
			MainMenu_Str_y_target = 60;
			//模拟左边弹出效果，进行选择控制提醒效果
			MainMenu_Rec_x = -8;
			MainMenu_Rec_x_target = 0;
			
			//进行蜂鸣器提醒或者其他操作
		}
	/*    限值代码段    */	
		if (MainMenu_Picture_x_target >= 42)//进行左边最后一张图片限值
		{
			MainMenu_Picture_x_target = 42;
		}
		else if (MainMenu_Picture_x_target <= (-MainMenu_MaxNum*64 + 42))//进行右闭最后一张图片限值
		{
			MainMenu_Picture_x_target = (-MainMenu_MaxNum*64 + 42);
		}
		if (MainMenu_Select < 0)//进行菜单选择值限值
		{
			MainMenu_Select = 0;
		}
		else if (MainMenu_Select >= MainMenu_MaxNum)
		{
			MainMenu_Select = MainMenu_MaxNum;
		}	
	}
	//对1级菜单坐标操作
	if (func_index_last >= Y1 && func_index_last <= Y6)
	{
		char temp = func_index_last - UI_Next_Addr;
		Str_AllArray[temp].select++;
	
		if (Str_AllArray[temp].select >= 0 && Str_AllArray[temp].select <= Str_AllArray[temp].len)
		{
			//进行蜂鸣器提醒或者其他操作
			Str_AllArray[temp].rate_y_target += Str_AllArray[temp].rate_y_duan;
			Str_AllArray[temp].fram_select = Str_AllArray[temp].select;
			
			if (Str_AllArray[temp].fram_select >= 0 && Str_AllArray[temp].fram_select <= 3)//是否对框下移动
			{
					Str_AllArray[temp].fram_y_target += 16;
			}
			else if (Str_AllArray[temp].select <= Str_AllArray[temp].len)//是否进行字符上移动
			{
				Str_AllArray[temp].str_y_target -= 16;
			}
			
			Str_AllArray[temp].fram_x_target = 7*Str_AllArray[temp].Str_Pa[Str_AllArray[temp].select].num;
		}
		
		if (Str_AllArray[temp].select > Str_AllArray[temp].len)	//	进行限值
		{
			Str_AllArray[temp].select = Str_AllArray[temp].len;
		}		
		
	}
	if (func_index == Y7 || func_index == Y8)//执行坐标加减函数后，在回到之前的索引值
	{
		func_index = func_index_last;
	}
	
}

/**
  * @brief  处理菜单“上一个/坐标减少”动作，主菜单中切换到左侧图标，二级菜单中上移选择项并更新选择框和滚动条目标坐标。
  * @param  无输入参数；函数通过全局 func_index_last、MainMenu_Select、MainMenu_* 和 Str_AllArray 修改菜单状态。
  * @return 无返回值。
  */
void Str_Coordinate_Decrease(void)
{
	if (func_index_last == Y0)//对主菜单坐标进行操作
	{
		MainMenu_Select -= 1;
		MainMenu_Picture_x_target += 64;
		if (MainMenu_Select >= 0 && MainMenu_Select <= MainMenu_MaxNum)
		{
			MainMenu_Str_y = 74;
			MainMenu_Str_y_target = 60;
			
			MainMenu_Rec_x = -8;
			MainMenu_Rec_x_target = 0;
			//进行蜂鸣器提醒或者其他操作
		}
	/*    限值代码段    */	
		if (MainMenu_Picture_x_target >= 42)//进行左边最后一张图片限值
		{
			MainMenu_Picture_x_target = 42;
		}
		else if (MainMenu_Picture_x_target <= (-MainMenu_MaxNum*64 + 42))//进行右闭最后一张图片限值
		{
			MainMenu_Picture_x_target = (-MainMenu_MaxNum*64 + 42);
		}
		if (MainMenu_Select < 0)//进行菜单选择值限值
		{
			MainMenu_Select = 0;
		}
		else if (MainMenu_Select >= MainMenu_MaxNum)
		{
			MainMenu_Select = MainMenu_MaxNum;
		}			
	}
	//对一级菜单坐标操作
	if (func_index_last >= Y1 && func_index_last <= Y6)
	{
		char temp = func_index_last - UI_Next_Addr;
		Str_AllArray[temp].select--;

		if (Str_AllArray[temp].select >= 0 && Str_AllArray[temp].select <= Str_AllArray[temp].len)
		{
			//进行蜂鸣器提醒或者其他操作
			Str_AllArray[temp].rate_y_target -= Str_AllArray[temp].rate_y_duan;
			Str_AllArray[temp].fram_select = Str_AllArray[temp].select;
			if (Str_AllArray[temp].fram_select >= 0 && Str_AllArray[temp].fram_select < 3)//是否对框上移动
			{
				Str_AllArray[temp].fram_y_target -= 16;
			}
			else if (Str_AllArray[temp].select > 0)//是否进行字符下移动
			{
				Str_AllArray[temp].str_y_target += 16;
			}
		}
		if (Str_AllArray[temp].select < 0)	//	进行限值
		{
			Str_AllArray[temp].select = 0;
		}			
	}
	
	if (func_index == Y7 || func_index == Y8)//执行坐标加减函数后，在回到之前的索引值
	{
		func_index = func_index_last;
	}
	
}

/**
  * @brief  绘制当前一级页面的二级字符串菜单，包括菜单项、复选框、右侧滚动条和反色选择框，并按目标坐标更新动画。
  * @param  无输入参数；函数根据全局 func_index 选择 Str_AllArray 中的数据源，通过 u8g2 绘制页面。
  * @return 无返回值。
  */
void String_show(void)
{
	char i = 0;
	char temp = func_index - UI_Next_Addr;
	Str_AllArray[temp].rate_y_duan = 64/(Str_AllArray[temp].len+1);
		
	u8g2_SetFont(&u8g2, u8g2_font_6x13_te);
   //显示字符
	for (i = 0; i <= Str_AllArray[temp].len; i++)
	{
		u8g2_DrawStr(&u8g2,Str_AllArray[temp].str_x,Str_AllArray[temp].str_y+i*16,Str_AllArray[temp].Str_Pa[i].str);
		u8g2_DrawFrame(&u8g2, 90, Str_AllArray[temp].str_y+i*16-8, 9, 9);//对应菜单下画空框
	}	
	
	
	//画选择条
	//u8g2_DrawVLine(&u8g2, 124, 0, (Str_AllArray[temp].len/4+1)*64);
	u8g2_DrawRBox(&u8g2, 122,Str_AllArray[temp].rate_y, 6,Str_AllArray[temp].rate_y_duan , 2);
	u8g2_DrawRFrame(&u8g2, 122, 1, 6, 62,2);
	u8g2_SetDrawColor(&u8g2, 2);
	//画选择框
	Str_AllArray[temp].fram_w_target = 7*Str_AllArray[temp].Str_Pa[Str_AllArray[temp].select].num;
	u8g2_DrawRBox(&u8g2,Str_AllArray[temp].fram_x,Str_AllArray[temp].fram_y,Str_AllArray[temp].fram_w,Str_AllArray[temp].fram_h,4);
	u8g2_SetDrawColor(&u8g2, 1);

	//画二级菜单确定框，扫码哪个一个被按下了，在这里可以加入自己想执行的代码
	for(i = 0; i <= Str_AllArray[temp].len; i++)
	{
		//测试案列
		if (temp == 1)
		{
			if ( 1 == Str_AllArray[temp].Str_Pa[i].flag)
			{
				u8g2_DrawBox(&u8g2, 90, Str_AllArray[temp].str_y+i*16-8, 9, 9);//对应按下操作的显示实心方框	

				switch(i)
				{
					case 0:
						Buzzer_State(1);
						break;
					case 1:
						Buzzer_State(1);
						break;
					default:
						Buzzer_State(1);
						Buzzer_State(1);
						break;
				}
			}
			else 
			{
				switch(i)
				{
					case 0:
						Buzzer_State(0);
						break;
					case 1:
						Buzzer_State(0);
						break;
				}

			}
		}	
	}

	run_str(&Str_AllArray[temp].str_y,&Str_AllArray[temp].str_y_target,speed,c_speed);
	run_str(&Str_AllArray[temp].fram_y,&Str_AllArray[temp].fram_y_target,speed,c_speed);
	run_str(&Str_AllArray[temp].fram_w,&Str_AllArray[temp].fram_w_target,3,1);
	run_str(&Str_AllArray[temp].rate_y,&Str_AllArray[temp].rate_y_target,speed,c_speed);
}

//	二级菜单操作各种选定执行,将对应一级菜单里面的MainSet_Str的flag就可以得知点击了哪个
/**
  * @brief  执行二级菜单确认动作，切换当前选中菜单项的 flag 状态，并将 func_index 恢复到确认前所在页面。
  * @param  无输入参数；函数通过全局 func_index_last、func_index 和 Str_AllArray 定位并修改当前选项。
  * @return 无返回值。
  */
void Str_Operate(void)
{
	int index = func_index_last - UI_Next_Addr;
	
	if(Str_AllArray[index].Str_Pa[Str_AllArray[index].select].flag == 1)
	{
		Str_AllArray[index].Str_Pa[Str_AllArray[index].select].flag = 0;
	}
	else if (Str_AllArray[index].Str_Pa[Str_AllArray[index].select].flag == 0)
	{
		Str_AllArray[index].Str_Pa[Str_AllArray[index].select].flag = 1;
	}
	//执行二级菜单操作函数后，在回到之前的索引值
	func_index = func_index_last;
}

/**
  * @brief  绘制游戏/图片页面，清屏后将 Wukong 128x64 位图直接显示到 OLED。
  * @param  无输入参数；函数通过全局 u8g2 和 Wukong 位图数组绘制，不接收可为空对象。
  * @return 无返回值。
  */
void Game_Display(void)
{
	u8g2_ClearBuffer(&u8g2);
	u8g2_DrawXBMP(&u8g2, 0, 0, 128, 64, Wukong);
	u8g2_SendBuffer(&u8g2);
}
