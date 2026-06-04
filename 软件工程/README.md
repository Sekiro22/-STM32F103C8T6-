# hWatch SE2 项目回忆笔记

这是一个基于 **STM32F103C8** 的手表/小型 OLED 交互界面工程，使用 **Keil MDK/uVision** 管理和编译。工程里集成了 STM32F10x 标准外设库、U8g2 图形库、按键中断、RTC 时钟、计时器、蜂鸣器、OLED 菜单和部分传感器驱动。

> 备注：源码里很多中文注释现在显示为乱码，大概率是当年使用 GBK/ANSI 编码保存，当前环境按 UTF-8 读取导致的。代码逻辑本身仍然能看。

## 快速入口

- Keil 工程文件：`Project.uvprojx`
- 目标芯片：`STM32F103C8`
- 输出文件名：`Project`
- 已开启 HEX 输出：`Objects/Project.hex`
- 入口文件：`User/main.c`
- 主控头文件：`User/main.h`

`main()` 的流程很短：

```c
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
```

也就是说，启动后初始化蜂鸣器、LED、按键、RTC、TIM2 计时器和 OLED，然后主循环不断执行菜单状态机并刷新屏幕。

## 目录结构

| 目录/文件 | 作用 |
| --- | --- |
| `User/` | 应用入口、STM32 中断模板、全局 include 配置 |
| `Hardware/` | 板级外设驱动：按键、LED、蜂鸣器、延时、BME/BMP280 |
| `System/` | 上层功能模块：OLED 适配、菜单、RTC 时钟、计时器、低功耗、图片资源 |
| `U8g2/` | U8g2 图形库源码 |
| `Library/` | STM32F10x 标准外设库 |
| `Start/` | Cortex-M3/STM32 启动文件和系统初始化 |
| `Objects/` | Keil 编译输出，包含 `Project.hex` |
| `Listings/` | Keil 编译列表/映射输出 |
| `DebugConfig/` | Keil 调试配置 |

## 硬件连接速记

从现有代码可读出的引脚分配：

| 功能 | 引脚/外设 | 说明 |
| --- | --- | --- |
| LED | `PC13` | 推挽输出，`LED_TurnState()` 翻转 |
| 按键 Enter/确认 | `PA0 / EXTI0` | 普通界面为确认；计时器界面为开始/暂停 |
| 按键 Up/上一个 | `PA1 / EXTI1` | 普通界面为上一个；计时器界面为清零 |
| 按键 Next/下一个 | `PA2 / EXTI2` | 下一个 |
| 按键 Back/返回 | `PA8 / EXTI8` | 返回；离开计时器时清 `Timer_Flag` |
| 按键 Home/主页 | `PA9 / EXTI9` | 回到时钟主页 |
| 蜂鸣器 | `PB8 / TIM4_CH3` | PWM 输出，约 330 Hz |
| OLED I2C | `PB10/PB11` | 代码里有硬件 I2C2 和软件 I2C 两套实现 |
| BMP280/BME280 I2C | `PA6/PA7` | 软件 I2C，地址 `0x77` |
| 计时器 | `TIM2` | 10 ms tick，用于秒表计数 |
| RTC | 内部 RTC | 当前配置走 LSI，备份寄存器 `BKP_DR1` 标记初始化 |

## OLED 和 U8g2

OLED 适配在 `System/OLED.c`。

代码里存在两套 OLED 初始化路径：

- 硬件 I2C2：`PB10/PB11`，`u8g2_Setup_sh1106_i2c_128x64_noname_f`
- 软件 I2C：`PB10/PB11`，`u8g2_Setup_ssd1306_i2c_128x64_noname_f`

需要注意一个历史遗留点：`System/OLED.h` 里定义的是 `IS_HUWEI`，但 `System/OLED.c` 里判断的是 `IS_HUAWEI`。因为宏名不一致，`IS_HUAWEI` 未定义时会按 0 处理，所以当前实际会进入硬件 I2C2/SH1106 分支。若要切换 OLED 驱动方式，先修正这个宏名。

菜单图标和图片资源主要在 `System/BMP_Lib.h`：

- `logo[7][300]`：主菜单 7 个图标
- `Wukong[]`：游戏页/图片页显示用的 128x64 位图

## 菜单系统

菜单核心在 `System/MEUN.c` 和 `System/MEUN.h`。整体是一个手写状态机：

- `Select_flag` 由按键中断设置。
- `Menu_Key_Set()` 在主循环中读取 `Select_flag`。
- `func_index` 表示当前状态。
- `Table[]` 是状态跳转表，包含 upper/next/enter/back 和当前状态渲染函数。
- 每次执行当前页面函数后调用 `u8g2_SendBuffer()` 和 `u8g2_ClearBuffer()`。

主菜单项：

1. `Setting`
2. `Clock`
3. `Timer`
4. `Calendar`
5. `Weather`
6. `Games`
7. `Power`

当前实际比较完整的功能：

- `Clock_Display()`：显示日期、星期、小时分钟秒。
- `Timer_Display()`：显示秒表，`PA0` 开始/暂停，`PA1` 清零。
- `Game_Display()`：显示 `Wukong` 图片。
- `Power_Disable()`：关闭 OLED，进入 STOP 模式。
- `String_show()` / `Str_Operate()`：二级菜单框架和选择动画，部分内容还是测试字符串。

菜单动画使用 `run_str()` 做坐标缓动，常见变量：

- `MainMenu_Picture_x_target`：主菜单图标目标位置。
- `MainMenu_Str_y_target`：菜单文字弹出动画目标位置。
- `Str_AllArray[].fram_*`：二级菜单选择框动画。
- `Str_AllArray[].rate_*`：右侧滚动条/进度条动画。

## 按键行为

按键初始化在 `Hardware/Key.c`，使用上拉输入 + 下降沿 EXTI 中断，并在中断里 `Delay_ms(20)` 做简单消抖。

`Select_flag` 编码：

| 值 | 含义 | 来源 |
| --- | --- | --- |
| `1` | upper/上一个 | `PA1` |
| `2` | next/下一个 | `PA2` |
| `3` | enter/确认 | `PA0` |
| `4` | back/返回 | `PA8` |
| `5` | home/主页 | `PA9` |

计时器界面比较特殊：

- `PA0` 不再进入菜单，而是启动/暂停 `TIM2`。
- `PA1` 清空 `TIMER_Structure`。
- `PA8/PA9` 离开计时器界面时清除 `Timer_Flag`。

## RTC 时钟

RTC 逻辑在 `System/MyRTC.c`。

- 默认初始时间：`2024-08-21 23:59:55`，星期值为 `4`。
- 当前 `#define LSI 1`，走内部 LSI。
- 首次初始化通过 `BKP_DR1 != 0xA5A5` 判断。
- `MyRTC_SetTime()` 使用 `mktime()` 转 Unix 时间戳，再减去 8 小时时区偏移。
- `MyRTC_ReadTime()` 读取 RTC counter，再加回 8 小时时区偏移。

如果以后要改初始时间，直接改 `RTC_SetTime` 的全局初始化值，或加一个时间设置菜单。

## 计时器/秒表

秒表在 `System/Timer.c`。

- `TIM2` 时钟来自 APB1。
- 分频 `6400 - 1`，自动重装 `100 - 1`。
- 按 72 MHz 系统时钟估算，中断周期约为 10 ms。
- `TIMER_Structure` 里 `Ms` 实际表示百分之一秒，不是真正毫秒。
- `Timer_Display()` 格式：`MM:SS:CC`。

注意：`TIM2` 初始化后只配置中断，没有在 `Timer_Init()` 里启动，进入秒表页后由按键控制 `TIM_Cmd(TIM2, ENABLE/DISABLE)`。

## 蜂鸣器

蜂鸣器在 `Hardware/Buzzer.c`。

- 使用 `TIM4_CH3`，输出到 `PB8`。
- `TIM_Period = 1000000 / 330 - 1`
- `TIM_Prescaler = 64 - 1`
- `Buzzer_State(1)` 打开 PWM，`Buzzer_State(0)` 关闭。

菜单二级选择里曾经预留了蜂鸣器开关逻辑，但目前看起来还在测试阶段。

## 低功耗

低功耗入口在 `System/PWR.c`：

```c
u8g2_ClearDisplay(&u8g2);
u8g2_SetPowerSave(&u8g2, 1);
PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
SystemInit();
```

进入 Power 菜单后会关闭 OLED 并进入 STOP 模式，唤醒后调用 `SystemInit()` 恢复系统时钟。`Clock_Display()` 中如果发现 `Power_Flag == 0`，会重新打开 OLED。

## BMP280/BME280

`Hardware/BME280.c/.h` 实现了一个 BMP280 风格的温压传感器驱动：

- 软件 I2C：`PA6 = SCL`，`PA7 = SDA`
- 地址：`0x77`
- 可读取温度和气压补偿值
- `Bmp_Init()` 会读取校准参数并配置过采样/滤波

但当前 `main()` 没有调用 `Bmp_Init()`，菜单里的 `Weather` 也还没有接上传感器显示逻辑，所以这部分更像是已写好但未整合完成的功能。

## 构建和烧录

1. 用 Keil MDK/uVision 打开 `Project.uvprojx`。
2. 确认目标为 `STM32F103C8`。
3. 编译 Target 1。
4. 输出 HEX 文件在 `Objects/Project.hex`。
5. 使用 ST-Link/J-Link/串口 ISP 等方式烧录 HEX。

工程包含路径已经配置在 Keil 中：

```text
.\Start;.\User;.\Library;.\Hardware;.\System;.\U8g2
```

预定义宏：

```text
USE_STDPERIPH_DRIVER
```

## 以后继续开发时优先看的文件

- `User/main.c`：系统启动顺序。
- `Hardware/Key.c`：按键和 `Select_flag` 来源。
- `System/MEUN.c`：菜单跳转、动画、页面调度。
- `System/Clock.c`：主表盘显示。
- `System/Timer.c`：秒表逻辑。
- `System/OLED.c`：OLED 驱动适配。
- `System/BMP_Lib.h`：菜单图标和位图。
- `Hardware/BME280.c`：温压传感器驱动。

## 明显的待整理点

- `MEUN` 应该是 `MENU` 的拼写误差，但当前文件名和 include 都依赖这个名字，改名需要全局同步。
- `IS_HUWEI` / `IS_HUAWEI` 宏名不一致，影响 OLED 分支选择。
- `Timer_Flag` 进入计时器页后只会置 1，离开时由部分按键清 0，后续可以整理成页面进入/退出钩子。
- `Timer.c` 里的 `Ms` 实际是 10 ms 计数，命名可改成 `Centisecond` 或 `Cs`。
- `BME280` 文件名和内部 `BMP280` 命名混用，实际代码更偏 BMP280。
- 二级菜单 `Str_AllArray` 里很多字符串还是测试数据，如 `aetting`、`cetting`。
- `Calendar`、`Weather`、`Games` 目前大多是占位或图片展示，功能还没完全接上。
- 中文注释建议统一转成 UTF-8，避免以后继续乱码。
