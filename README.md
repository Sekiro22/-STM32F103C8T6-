# hWatch STM32F103C8T6 手表项目

这是一个基于 **STM32F103C8T6** 的小型手表项目仓库，包含硬件生产资料和 Keil 软件工程。仓库当前更适合作为项目归档和后续二次开发入口：硬件资料可以用于查看原理图、打样 PCB 和整理 BOM，软件工程可以直接用 Keil MDK 打开继续维护。

## 仓库内容

| 目录 | 内容 | 用途 |
| --- | --- | --- |
| [`软件工程/`](./软件工程/) | Keil MDK 工程、STM32F10x 标准外设库、U8g2、业务源码、当前 HEX 输出 | 编译、烧录、继续开发固件 |
| [`原理图/`](./原理图/) | `SCH_Schematic1_2024-08-15.pdf` | 查看电路连接和器件关系 |
| [`Gerber文件/`](./Gerber文件/) | `Gerber_PCB5_2024-08-15.zip` | PCB 打样生产文件 |
| [`BOM表/`](./BOM表/) | `BOM_Board1_PCB5_2024-08-15.xlsx` | 元器件采购和焊接核对 |
| [`嘉立创EDA源文件/`](./嘉立创EDA源文件/) | `hWatch_2024-08-12_16-13-03.zip` | 在嘉立创 EDA 中继续修改硬件设计 |

## 软件工程概览

软件工程位于 [`软件工程/`](./软件工程/)，详细说明见 [`软件工程/README.md`](./软件工程/README.md)。

核心入口：

- Keil 工程：[`软件工程/Project.uvprojx`](./软件工程/Project.uvprojx)
- 程序入口：[`软件工程/User/main.c`](./软件工程/User/main.c)
- 菜单状态机：[`软件工程/System/MEUN.c`](./软件工程/System/MEUN.c)
- 主时钟界面：[`软件工程/System/Clock.c`](./软件工程/System/Clock.c)
- 按键中断：[`软件工程/Hardware/Key.c`](./软件工程/Hardware/Key.c)
- OLED 适配：[`软件工程/System/OLED.c`](./软件工程/System/OLED.c)

当前固件主要包含：

- RTC 时钟主界面；
- U8g2 OLED 菜单界面；
- 按键中断驱动的菜单切换；
- 秒表页面；
- 蜂鸣器 PWM 控制；
- OLED 低功耗关闭和 STOP 模式入口；
- BMP280/BME280 温压传感器驱动代码，但 Weather 页面尚未完整接入显示。

## 快速使用

### 编译和烧录固件

1. 进入 [`软件工程/`](./软件工程/)。
2. 使用 Keil MDK/uVision 打开 `Project.uvprojx`。
3. 确认目标芯片为 `STM32F103C8`。
4. 编译 `Target 1`。
5. 使用生成的 [`软件工程/Objects/Project.hex`](./软件工程/Objects/Project.hex) 烧录到开发板。

### 查看或修改硬件

1. 先查看 [`原理图/SCH_Schematic1_2024-08-15.pdf`](./原理图/SCH_Schematic1_2024-08-15.pdf) 理解电路。
2. 需要打板时使用 [`Gerber文件/Gerber_PCB5_2024-08-15.zip`](./Gerber文件/Gerber_PCB5_2024-08-15.zip)。
3. 需要采购或核对器件时查看 [`BOM表/BOM_Board1_PCB5_2024-08-15.xlsx`](./BOM表/BOM_Board1_PCB5_2024-08-15.xlsx)。
4. 需要继续改 PCB 时，导入 [`嘉立创EDA源文件/hWatch_2024-08-12_16-13-03.zip`](./嘉立创EDA源文件/hWatch_2024-08-12_16-13-03.zip)。

## 代码维护约定

- 自写业务代码使用中文 Doxygen 风格注释。
- 函数注释包含 `@brief`、`@param`、`@return`。
- 第三方库和 STM32 标准外设库尽量保持原样，避免后续升级困难。
- 修改 Keil 工程目录时要同步检查 `Project.uvprojx` 中的相对路径。
- 当前部分早期注释存在编码历史问题，后续维护以新增 UTF-8 注释和当前代码实现为准。

## 后续开发建议

- 修正 `System/MEUN.*` 文件名拼写，若修改需要同步 Keil 工程引用。
- 修正 `System/OLED.h` 与 `System/OLED.c` 中 `IS_HUWEI` / `IS_HUAWEI` 宏名不一致问题。
- 将 Weather 页面接入 BMP280/BME280 温度、气压读取。
- 清理二级菜单中的测试字符串和占位页面。
- 给硬件资料目录补充更具体的版本说明和生产注意事项。
