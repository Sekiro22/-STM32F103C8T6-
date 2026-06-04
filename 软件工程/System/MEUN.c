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

void Game_Display(void)
{
	u8g2_ClearBuffer(&u8g2);
	u8g2_DrawXBMP(&u8g2, 0, 0, 128, 64, Wukong);
	u8g2_SendBuffer(&u8g2);
}
