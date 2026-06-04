#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "BME280.h"

/*引脚配置层*/

/**
  * @brief  设置 BMP280 软件 I2C 的 SCL 引脚 PA6 电平。
  * @param  BitValue 输入参数；0 表示拉低 PA6，非 0 表示置高 PA6，不是指针不允许为空。
  * @return 无返回值。
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)BitValue);		//根据BitValue，设置SCL引脚的电平
//	Delay_us(10);												//延时10us，防止时序频率超过要求
}

/**
  * @brief  设置 BMP280 软件 I2C 的 SDA 引脚 PA7 电平。
  * @param  BitValue 输入参数；0 表示拉低 PA7，非 0 表示置高 PA7，不是指针不允许为空。
  * @return 无返回值。
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)BitValue);		//根据BitValue，设置SDA引脚的电平，BitValue要实现非0即1的特性
//	Delay_us(10);												//延时10us，防止时序频率超过要求
}

/**
  * @brief  读取 BMP280 软件 I2C 的 SDA 引脚 PA7 当前输入电平。
  * @param  无输入参数；函数固定读取 GPIOA Pin7，不接收外部指针。
  * @return 返回 0 表示 SDA 当前为低电平，返回 1 表示 SDA 当前为高电平；当前实现没有错误码。
  */
uint8_t MyI2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);		//读取SDA电平
//	Delay_us(10);												//延时10us，防止时序频率超过要求
	return BitValue;											//返回SDA电平
}

/**
  * @brief  初始化 BMP280 软件 I2C 引脚，将 PA6/PA7 配置为开漏输出并默认释放为高电平。
  * @param  无输入参数；函数固定配置 GPIOA、PA6 和 PA7，不接收外部配置对象。
  * @return 无返回值。
  */
void MyI2C_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PB10和PB11引脚初始化为开漏输出
	
	/*设置默认电平*/
	GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);			//设置PB10和PB11引脚初始化后默认为高电平（释放总线状态）
}

/*协议层*/

/**
  * @brief  产生软件 I2C 起始条件，先释放 SCL/SDA，再在 SCL 高电平期间拉低 SDA 并拉低 SCL 占用总线。
  * @param  无输入参数；函数直接操作 PA6/PA7，不接收外部对象。
  * @return 无返回值。
  */
void MyI2C_Start(void)
{
	MyI2C_W_SDA(1);							//释放SDA，确保SDA为高电平
	MyI2C_W_SCL(1);							//释放SCL，确保SCL为高电平
	MyI2C_W_SDA(0);							//在SCL高电平期间，拉低SDA，产生起始信号
	MyI2C_W_SCL(0);							//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * @brief  产生软件 I2C 停止条件，在 SCL 高电平期间释放 SDA。
  * @param  无输入参数；函数直接操作 PA6/PA7，不接收外部对象。
  * @return 无返回值。
  */
void MyI2C_Stop(void)
{
	MyI2C_W_SDA(0);							//拉低SDA，确保SDA为低电平
	MyI2C_W_SCL(1);							//释放SCL，使SCL呈现高电平
	MyI2C_W_SDA(1);							//在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * @brief  按高位在前的顺序通过软件 I2C 发送 1 个字节。
  * @param  Byte 输入参数；待发送的 8 位数据，不是指针不允许为空，取值范围为 0x00~0xFF。
  * @return 无返回值。
  */
void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)				//循环8次，主机依次发送数据的每一位
	{
		MyI2C_W_SDA(Byte & (0x80 >> i));	//使用掩码的方式取出Byte的指定一位数据并写入到SDA线
		MyI2C_W_SCL(1);						//释放SCL，从机在SCL高电平期间读取SDA
		MyI2C_W_SCL(0);						//拉低SCL，主机开始发送下一位数据
	}
}

/**
  * @brief  通过软件 I2C 按高位在前的顺序接收 1 个字节。
  * @param  无输入参数；函数直接读取 SDA 引脚，不接收外部缓冲区。
  * @return 返回接收到的 8 位数据，取值范围为 0x00~0xFF；当前实现没有超时和错误码。
  */
uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;					//定义接收的数据，并赋初值0x00，此处必须赋初值0x00，后面会用到
	MyI2C_W_SDA(1);							//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	for (i = 0; i < 8; i ++)				//循环8次，主机依次接收数据的每一位
	{
		MyI2C_W_SCL(1);						//释放SCL，主机机在SCL高电平期间读取SDA
		if (MyI2C_R_SDA() == 1){Byte |= (0x80 >> i);}	//读取SDA数据，并存储到Byte变量
														//当SDA为1时，置变量指定位为1，当SDA为0时，不做处理，指定位为默认的初值0
		MyI2C_W_SCL(0);						//拉低SCL，从机在SCL低电平期间写入SDA
	}
	return Byte;							//返回接收到的一个字节数据
}

/**
  * @brief  通过软件 I2C 发送应答位或非应答位。
  * @param  AckBit 输入参数；0 表示应答 ACK，1 表示非应答 NACK，其他非 0 值按高电平处理；参数不是指针不允许为空。
  * @return 无返回值。
  */
void MyI2C_SendAck(uint8_t AckBit)
{
	MyI2C_W_SDA(AckBit);					//主机把应答位数据放到SDA线
	MyI2C_W_SCL(1);							//释放SCL，从机在SCL高电平期间，读取应答位
	MyI2C_W_SCL(0);							//拉低SCL，开始下一个时序模块
}

/**
  * @brief  通过软件 I2C 读取从设备返回的应答位。
  * @param  无输入参数；函数释放 SDA 后读取引脚状态，不接收外部对象。
  * @return 返回 0 表示收到 ACK，返回 1 表示收到 NACK；当前实现没有超时和错误码。
  */
uint8_t MyI2C_ReceiveAck(void)
{
	uint8_t AckBit;							//定义应答位变量
	MyI2C_W_SDA(1);							//接收前，主机先确保释放SDA，避免干扰从机的数据发送
	MyI2C_W_SCL(1);							//释放SCL，主机机在SCL高电平期间读取SDA
	AckBit = MyI2C_R_SDA();					//将应答位存储到变量里
	MyI2C_W_SCL(0);							//拉低SCL，开始下一个时序模块
	return AckBit;							//返回定义应答位变量
}

//设置BMP过采样因子 MODE 
//BMP280_SLEEP_MODE||BMP280_FORCED_MODE||BMP280_NORMAL_MODE
/**
  * @brief  根据过采样配置结构体写入 BMP280 控制测量寄存器，设置温度过采样、气压过采样和工作模式。
  * @param  Oversample_Mode 输入参数；指向过采样配置结构体，不允许为空；字段取值应来自 BMP280_P_OVERSAMPLING、BMP280_T_OVERSAMPLING 和 BMP280_WORK_MODE 枚举。
  * @return 无返回值。
  */
void BMP280_Set_TemOversamp(BMP_OVERSAMPLE_MODE * Oversample_Mode)
{
	u8 Regtmp;
	Regtmp = ((Oversample_Mode->T_Osample)<<5)|
			 ((Oversample_Mode->P_Osample)<<2)|
			 ((Oversample_Mode)->WORKMODE);
	
	BMP280_Write_Byte(BMP280_CTRLMEAS_REG,Regtmp);
}


//设置保持时间和滤波器分频因子
/**
  * @brief  根据配置结构体写入 BMP280 配置寄存器，设置待机时间、IIR 滤波系数和 SPI 使能位。
  * @param  BMP_Config 输入参数；指向 BMP280 配置结构体，不允许为空；字段取值应来自 BMP280_T_SB、BMP280_FILTER_COEFFICIENT 和 ENABLE/DISABLE。
  * @return 无返回值。
  */
void BMP280_Set_Standby_FILTER(BMP_CONFIG * BMP_Config)
{
	u8 Regtmp;
	Regtmp = ((BMP_Config->T_SB)<<5)|
			 ((BMP_Config->FILTER_COEFFICIENT)<<2)|
			 ((BMP_Config->SPI_EN));
	
	BMP280_Write_Byte(BMP280_CONFIG_REG,Regtmp);
}

//获取BMP当前状态
//status_flag = BMP280_MEASURING ||
//			 	BMP280_IM_UPDATE
/**
  * @brief  读取 BMP280 状态寄存器并判断指定状态位是否置位。
  * @param  status_flag 输入参数；待检测的状态位掩码，不是指针不允许为空，常用取值为 BMP280_MEASURING 或 BMP280_IM_UPDATE。
  * @return 返回 SET 表示指定状态位为 1，返回 RESET 表示指定状态位为 0；当前实现没有通信错误码。
  */
u8  BMP280_GetStatus(u8 status_flag)
{
	u8 flag;
	flag = BMP280_Read_Byte(BMP280_STATUS_REG);
	if(flag&status_flag)	return SET;
	else return RESET;
}

/*******************主要部分*********************/
/****************获取传感器精确值****************/
//大气压值-Pa
/**
  * @brief  读取 BMP280 原始气压 ADC 数据并调用补偿函数换算为气压值。
  * @param  无输入参数；函数通过固定 BMP280 寄存器地址读取数据，不接收外部缓冲区。
  * @return 返回补偿后的气压数值；当前定点补偿分支返回 Q24.8 格式转换后的数值，通信失败不会产生独立错误码。
  */
double BMP280_Get_Pressure(void)
{
	uint8_t XLsb,Lsb, Msb;
	long signed Bit32;
	double pressure;
	XLsb = BMP280_Read_Byte(BMP280_PRESSURE_XLSB_REG);
	Lsb	 = BMP280_Read_Byte(BMP280_PRESSURE_LSB_REG);
	Msb	 = BMP280_Read_Byte(BMP280_PRESSURE_MSB_REG);
	Bit32 = ((long)(Msb << 12))|((long)(Lsb << 4))|(XLsb>>4);	//寄存器的值,组成一个浮点数
	pressure = bmp280_compensate_P_int64(Bit32);
	return pressure;
}

//温度值-℃
/**
  * @brief  读取 BMP280 原始温度 ADC 数据并调用补偿函数换算为温度值。
  * @param  无输入参数；函数通过固定 BMP280 寄存器地址读取数据，不接收外部缓冲区。
  * @return 返回补偿后的温度数值；当前定点补偿分支单位为 0.01 摄氏度，通信失败不会产生独立错误码。
  */
double BMP280_Get_Temperature(void)
{
	uint8_t XLsb,Lsb, Msb;
	long signed Bit32;
	double temperature;
	XLsb = BMP280_Read_Byte(BMP280_TEMPERATURE_XLSB_REG);
	Lsb	 = BMP280_Read_Byte(BMP280_TEMPERATURE_LSB_REG);
	Msb	 = BMP280_Read_Byte(BMP280_TEMPERATURE_MSB_REG);
	Bit32 = ((long)(Msb << 12))|((long)(Lsb << 4))|(XLsb>>4);	//寄存器的值,组成一个浮点数
	temperature = bmp280_compensate_T_int32(Bit32);
	return temperature;
}
/***************************************END OF LINE*********************************************/

BMP280 bmp280_inst;
BMP280* bmp280 = &bmp280_inst;		//这个全局结构体变量用来保存存在芯片内ROM补偿参数
/**
  * @brief  初始化 BMP280 传感器，初始化软件 I2C、读取温度和气压校准参数、软复位芯片并配置过采样、工作模式和滤波参数。
  * @param  无输入参数；函数使用全局 bmp280 指针保存校准参数，不接收外部配置对象。
  * @return 无返回值。
  */
void Bmp_Init(void)
{
	MyI2C_Init();
	u8 Lsb,Msb;
	
	/********************接下来读出矫正参数*********************/
	//温度传感器的矫正值
	Lsb = BMP280_Read_Byte(BMP280_DIG_T1_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_T1_MSB_REG);
	bmp280->T1 = (((u16)Msb)<<8) + Lsb;			//高位加低位
	Lsb = BMP280_Read_Byte(BMP280_DIG_T2_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_T2_MSB_REG);
	bmp280->T2 = (((u16)Msb)<<8) + Lsb;		
	Lsb = BMP280_Read_Byte(BMP280_DIG_T3_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_T3_MSB_REG);
	bmp280->T3 = (((u16)Msb)<<8) + Lsb;		
	
	//大气压传感器的矫正值
	Lsb = BMP280_Read_Byte(BMP280_DIG_P1_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P1_MSB_REG);
	bmp280->P1 = (((u16)Msb)<<8) + Lsb;		
	Lsb = BMP280_Read_Byte(BMP280_DIG_P2_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P2_MSB_REG);
	bmp280->P2 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P3_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P3_MSB_REG);
	bmp280->P3 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P4_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P4_MSB_REG);
	bmp280->P4 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P5_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P5_MSB_REG);
	bmp280->P5 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P6_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P6_MSB_REG);
	bmp280->P6 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P7_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P7_MSB_REG);
	bmp280->P7 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P8_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P8_MSB_REG);
	bmp280->P8 = (((u16)Msb)<<8) + Lsb;	
	Lsb = BMP280_Read_Byte(BMP280_DIG_P9_LSB_REG);
	Msb = BMP280_Read_Byte(BMP280_DIG_P9_MSB_REG);
	bmp280->P9 = (((u16)Msb)<<8) + Lsb;	
	/******************************************************/
	BMP280_Write_Byte(BMP280_RESET_REG,BMP280_RESET_VALUE);	//往复位寄存器写入给定值
	
	BMP_OVERSAMPLE_MODE			BMP_OVERSAMPLE_MODEStructure;
	BMP_OVERSAMPLE_MODEStructure.P_Osample = BMP280_P_MODE_3;
	BMP_OVERSAMPLE_MODEStructure.T_Osample = BMP280_T_MODE_1;
	BMP_OVERSAMPLE_MODEStructure.WORKMODE  = BMP280_NORMAL_MODE;
	BMP280_Set_TemOversamp(&BMP_OVERSAMPLE_MODEStructure);
	
	BMP_CONFIG					BMP_CONFIGStructure;
	BMP_CONFIGStructure.T_SB = BMP280_T_SB1;
	BMP_CONFIGStructure.FILTER_COEFFICIENT = BMP280_FILTER_MODE_4;
	BMP_CONFIGStructure.SPI_EN = DISABLE;
	
	BMP280_Set_Standby_FILTER(&BMP_CONFIGStructure);
}

/**
  * @brief  通过软件 I2C 向 BMP280 指定寄存器写入 1 字节数据。
  * @param  RegAddress 输入参数；目标寄存器地址，不是指针不允许为空，取值应为 BMP280 支持的寄存器地址。
  * @param  Data 输入参数；待写入的数据字节，不是指针不允许为空，取值范围为 0x00~0xFF。
  * @return 无返回值。
  */
void BMP280_Write_Byte(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(BMP280_ADDRESS<<1);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_Stop();						//I2C终止
}

/**
  * @brief  通过软件 I2C 从 BMP280 指定寄存器读取 1 字节数据。
  * @param  RegAddress 输入参数；目标寄存器地址，不是指针不允许为空，取值应为 BMP280 支持的寄存器地址。
  * @return 返回读取到的 8 位寄存器值，取值范围为 0x00~0xFF；当前实现没有通信失败错误码。
  */
uint8_t BMP280_Read_Byte(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(BMP280_ADDRESS<<1);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(BMP280_ADDRESS<<1 | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					//接收应答
	Data = MyI2C_ReceiveByte();			//接收指定寄存器的数据
	MyI2C_SendAck(1);					//发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						//I2C终止
	
	return Data;
}

/**************************传感器值转定点值*************************************/
BMP280_S32_t t_fine;			//用于计算补偿
//我用浮点补偿
#define USE_FIXED_POINT_COMPENSATE 1
#ifdef USE_FIXED_POINT_COMPENSATE
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC. 
// t_fine carries fine temperature as global value
/**
  * @brief  使用 BMP280 官方定点公式对原始温度 ADC 值进行补偿，并更新全局 t_fine。
  * @param  adc_T 输入参数；BMP280 原始温度 ADC 值，不是指针不允许为空，应来自温度数据寄存器组合结果。
  * @return 返回温度补偿结果，单位为 0.01 摄氏度；例如返回 5123 表示 51.23 摄氏度，当前实现没有失败错误码。
  */
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
	BMP280_S32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) * 
	((BMP280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
/**
  * @brief  使用 BMP280 官方 64 位定点公式和全局 t_fine 对原始气压 ADC 值进行补偿。
  * @param  adc_P 输入参数；BMP280 原始气压 ADC 值，不是指针不允许为空，应来自气压数据寄存器组合结果。
  * @return 返回 Q24.8 格式的气压补偿值；当补偿分母为 0 时返回 0 以避免除零，当前实现没有其他错误码。
  */
BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P)
{
	BMP280_S64_t var1, var2, p;
	var1 = ((BMP280_S64_t)t_fine) - 128000;
	var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
	var2 = var2 + ((var1*(BMP280_S64_t)dig_P5)<<17);
	var2 = var2 + (((BMP280_S64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3)>>8) + ((var1 * (BMP280_S64_t)dig_P2)<<12);
	var1 = (((((BMP280_S64_t)1)<<47)+var1))*((BMP280_S64_t)dig_P1)>>33;
	if (var1 == 0)
	{
	return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((BMP280_S64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((BMP280_S64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7)<<4);
	return (BMP280_U32_t)p;
}


/***********************************CUT*************************************/
#else
/**************************传感器值转定点值*************************************/
// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
/**
  * @brief  使用 BMP280 官方浮点公式对原始温度 ADC 值进行补偿，并更新全局 t_fine。
  * @param  adc_T 输入参数；BMP280 原始温度 ADC 值，不是指针不允许为空，应来自温度数据寄存器组合结果。
  * @return 返回摄氏度温度值；当前实现没有失败错误码。
  */
double bmp280_compensate_T_double(BMP280_S32_t adc_T)
{
	double var1, var2, T;
	var1 = (((double)adc_T)/16384.0 - ((double)dig_T1)/1024.0) * ((double)dig_T2);
	var2 = ((((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0) *
	(((double)adc_T)/131072.0 - ((double) dig_T1)/8192.0)) * ((double)dig_T3);
	t_fine = (BMP280_S32_t)(var1 + var2);
	T = (var1 + var2) / 5120.0;
	return T;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
/**
  * @brief  使用 BMP280 官方浮点公式和全局 t_fine 对原始气压 ADC 值进行补偿。
  * @param  adc_P 输入参数；BMP280 原始气压 ADC 值，不是指针不允许为空，应来自气压数据寄存器组合结果。
  * @return 返回 Pa 单位气压值；当补偿分母为 0 时返回 0 以避免除零，当前实现没有其他错误码。
  */
double bmp280_compensate_P_double(BMP280_S32_t adc_P)
{
	double var1, var2, p;
	var1 = ((double)t_fine/2.0) - 64000.0;
	var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
	var2 = var2 + var1 * ((double)dig_P5) * 2.0;
	var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
	var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
	if (var1 == 0.0)
	{
	return 0; // avoid exception caused by division by zero
	}
	p = 1048576.0 - (double)adc_P;
	p = (p - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double)dig_P9) * p * p / 2147483648.0;
	var2 = p * ((double)dig_P8) / 32768.0;
	p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
	return p;
}
#endif

