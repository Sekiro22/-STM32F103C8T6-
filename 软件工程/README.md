# hWatch SE2 软件工程说明

这是一个基于 **STM32F103C8** 的手表/小型 OLED 交互界面工程，使用 **Keil MDK/uVision** 管理和编译。工程集成 STM32F10x 标准外设库、U8g2 图形库、按键中断、RTC 时钟、秒表、蜂鸣器、OLED 菜单、低功耗和 BMP280/BME280 传感器驱动。

> 说明：部分旧中文注释由于历史编码问题已经乱码。后续维护以当前代码实现和新增 Doxygen 注释为准。

## 快速入口

| 项目 | 内容 |
| --- | --- |
| Keil 工程 | `Project.uvprojx` |
| 目标芯片 | `STM32F103C8` |
| 输出文件 | `Objects/Project.hex` |
| 程序入口 | `User/main.c` |
| 菜单核心 | `System/MEUN.c` |
| 主表盘 | `System/Clock.c` |
| 按键输入 | `Hardware/Key.c` |

启动流程：

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

主循环只调用 `Menu_Key_Set()`。按键中断负责设置 `Select_flag`，菜单调度函数负责消费按键事件、切换 `func_index`、调用对应页面绘制函数，并刷新 OLED。

## 目录结构

| 目录/文件 | 作用 |
| --- | --- |
| `User/` | 应用入口、STM32 中断模板、工程公共 include |
| `Hardware/` | 自写板级驱动：按键、LED、蜂鸣器、延时、BMP280/BME280 |
| `System/` | 自写系统功能：OLED 适配、菜单、RTC、秒表、低功耗、位图资源 |
| `U8g2/` | U8g2 第三方图形库源码 |
| `Library/` | STM32F10x 标准外设库 |
| `Start/` | 启动文件、CMSIS 和系统初始化 |
| `Objects/` | Keil 编译输出，当前保留 `Project.hex` |
| `DebugConfig/` | Keil 调试配置 |

## 硬件连接

| 功能 | 引脚/外设 | 当前用途 |
| --- | --- | --- |
| LED | `PC13` | 状态灯，支持翻转 |
| 确认键 | `PA0 / EXTI0` | 普通页面确认；秒表页开始/暂停 |
| 上一个键 | `PA1 / EXTI1` | 菜单上一个；秒表页清零 |
| 下一个键 | `PA2 / EXTI2` | 菜单下一个 |
| 返回键 | `PA8 / EXTI8` | 返回上级页面 |
| 主页键 | `PA9 / EXTI9` | 返回时钟主界面 |
| 蜂鸣器 | `PB8 / TIM4_CH3` | PWM 输出，约 330 Hz |
| OLED | `PB10/PB11` | I2C，当前代码实际进入硬件 I2C2/SH1106 分支 |
| BMP280/BME280 | `PA6/PA7` | 软件 I2C，地址 `0x77` |
| 秒表 tick | `TIM2` | 10 ms 周期中断 |
| RTC | 内部 RTC | 当前使用 LSI，`BKP_DR1` 标记是否首次初始化 |

## 菜单实现

菜单采用“按键事件 + 状态表 + 页面函数”的结构。

```text
按键中断
  -> 设置 Select_flag
主循环 Menu_Key_Set()
  -> 根据 Select_flag 查询 Table[]
  -> 更新 func_index
  -> 调用当前页面函数
  -> u8g2_SendBuffer()
  -> u8g2_ClearBuffer()
```

核心变量：

| 变量 | 说明 |
| --- | --- |
| `Select_flag` | 按键事件编号，由 EXTI 中断写入 |
| `func_index` | 当前页面/动作节点编号 |
| `Table[]` | 状态跳转表，定义 upper/next/enter/back 和页面函数 |
| `MainMenu_Select` | 主菜单当前选中的功能项 |
| `MainMenu_Picture_x_target` | 主菜单图标组目标 X 坐标 |
| `Str_AllArray[]` | 二级菜单文字、选择框、滚动条和选中状态数据 |

主菜单项：

1. `Setting`
2. `Clock`
3. `Timer`
4. `Calendar`
5. `Weather`
6. `Games`
7. `Power`

主菜单图标来自 `System/BMP_Lib.h` 的 `logo[7][300]`。`ui_show()` 将 7 个图标按 64 像素间距横向绘制，通过调整 `MainMenu_Picture_x_target` 实现滑动动画。文字使用 `u8g2_GetStrWidth()` 计算宽度后居中。

二级菜单由 `String_show()` 绘制，支持：

- 菜单文字列表；
- 右侧滚动条；
- 当前选中项反色选择框；
- 选择框宽度和位置动画；
- `Str_Operate()` 切换当前选项的 `flag`。

## 主要功能模块

### 时钟

`System/Clock.c` 的 `Clock_Display()` 是默认主界面。它读取 `RTC_SetTime`，绘制日期、星期、小时分钟和秒数。低功耗唤醒后也会在这里重新打开 OLED。

### RTC

`System/MyRTC.c` 使用 STM32 内部 RTC。首次运行时写入默认时间并在 `BKP_DR1` 写入 `0xA5A5`，后续启动只同步和读取 RTC。当前默认使用 LSI，时间戳计算中显式处理 UTC+8 偏移。

### 秒表

`System/Timer.c` 使用 TIM2 产生 10 ms 中断。`TIMER_Structure.Ms` 实际表示百分之一秒计数，显示格式为 `MM:SS:CC`。进入秒表页面后，`PA0` 控制启动/暂停，`PA1` 清零。

### OLED

`System/OLED.c` 提供 U8g2 与 STM32 I2C 的适配层。当前存在历史宏名问题：头文件定义 `IS_HUWEI`，源文件判断 `IS_HUAWEI`。由于 `IS_HUAWEI` 未定义时按 0 处理，实际会编译硬件 I2C2/SH1106 分支。

### BMP280/BME280

`Hardware/BME280.c` 实现软件 I2C、BMP280 寄存器读写、校准参数读取、温度和气压补偿。当前 `main()` 尚未调用 `Bmp_Init()`，Weather 页面也尚未接入传感器显示。

### 低功耗

`System/PWR.c` 的 `Power_Disable()` 会清屏、关闭 OLED、进入 STOP 模式，唤醒后调用 `SystemInit()` 恢复系统时钟。

## 构建与烧录

1. 使用 Keil MDK/uVision 打开 `Project.uvprojx`。
2. 确认目标为 `STM32F103C8`。
3. 编译 `Target 1`。
4. 生成的 HEX 位于 `Objects/Project.hex`。
5. 使用 ST-Link/J-Link/串口 ISP 烧录。

工程包含路径：

```text
.\Start;.\User;.\Library;.\Hardware;.\System;.\U8g2
```

预定义宏：

```text
USE_STDPERIPH_DRIVER
```

## 注释维护规范

自写函数使用中文 Doxygen 风格注释：

```c
/**
  * @brief  根据当前 RTC 时间绘制时钟主界面，并在低功耗唤醒后恢复 OLED 显示。
  * @param  无输入参数；函数通过全局 RTC_SetTime 和 u8g2 访问当前时间与显示缓冲区，不允许传入空对象。
  * @return 无返回值。
  */
```

要求：

- `@brief` 必须描述当前函数体实际做的事。
- 每个形参必须单独使用 `@param` 描述含义、输入/输出属性、是否允许为空和关键取值约束。
- 无形参函数写清“无输入参数”。
- `@return` 必须说明返回值含义；`void` 函数写“无返回值”。
- 不修改代码逻辑时，只更新注释和 README。

## 后续建议

- 修正 `MEUN` 文件名拼写需要谨慎同步 include 和 Keil 工程配置。
- 修正 `IS_HUWEI` / `IS_HUAWEI` 宏名不一致问题。
- 将 `Timer_Flag` 这类页面状态整理成明确的页面进入/退出流程。
- 将 Weather 页面接入 `Bmp_Init()`、温度和气压读取。
- 清理二级菜单中的测试字符串，如 `aetting`、`cetting`。
- 将仍然乱码的历史注释逐步替换为 UTF-8 中文 Doxygen 注释。
