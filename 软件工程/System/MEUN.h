#ifndef __MEUN_H__
#define __MEUN_H__

#define UI_Next_Addr		2

#define Y0		1		//填UI所在页  也就是UI_Next_Addr - 1
#define Y1		2		//填一级菜单开始页
#define Y6		8		//填一级菜单最后一页
#define Y7		9		//填坐标位置右移页
#define Y8		10		//填坐标位置左移页

/*  菜单选择操作代码段  */
typedef struct
{
	char* str;
	char num;
	int flag;
}MainSet_Str;

typedef struct
{
	MainSet_Str Str_Pa[10];//二级菜单最大容量
	int select;//二级菜单的选择值
	int len;//从0开始算起有几个字符串
	int str_x;//开始字符串x轴的起始位置
	int str_x_target;
	int str_y;
	int str_y_target;
	int fram_select;//选择框的选择值
	int fram_x;
	int fram_x_target;
	int fram_y;
	int fram_y_target;	
	int fram_h;
	int fram_w;
	int fram_w_target;
	int rate_y;//选择进度条的长度
	int rate_y_target;
	int rate_y_duan;//选择进度条的有几段，比如说有一米长的线条，有几个元素就分为几段，从而达到不同菜单下的长短
}MainSet_Str_All;


typedef struct
{
	char current;
	char upper;
	char next;
	char enter;
	char back;
	void (*current_operation)(void);
}Menu_Operate;

void Menu_Key_Set(void);

void ui_show(void);
void Str_Coordinate_Add(void);
void Str_Coordinate_Decrease(void);
void String_show(void);
void Str_Operate(void);

void Game_Display(void);

#endif
