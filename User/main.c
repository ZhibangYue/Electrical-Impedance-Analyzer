#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "OLED.h"
#include "AD.h"
#include "Delay.h"
#include "AD9833.h"
#include "Serial.h"
#include "tjc_usart_hmi.h"
#include "AD7606.h"
#include "relay.h"
#include "LC.h"



// 串口屏发送的数据长度，这里为6
#define FRAMELENGTH 6
// AD7606的量程
#define RANGE 5000.0

uint16_t ADValue;
uint16_t i = 0;
char str[100];
uint8_t temp; // AD7606的BUSY状态
float v0 = 0; // 初始电压
float v, v1 = 0;
float ri, ro, a, ui1, uo1 = 0.0;
long vf = 0;			  // 截止频率
int16_t DB_data[8] = {0}; // AD7606的数据

float Measure_u(uint8_t adc, uint8_t method);
long Change_freq(void);

void Measure_LC(uint16_t freq);
void Measure_LC2(uint16_t freq);
float cosc(float a, float b, float c);
int main(void)
{	
	OLED_Init();
	Serial_Init();
	// AD9833发波
	AD9833_Init_GPIO();
	AD9833_WaveSeting(500.0, 0, SIN_WAVE, 0);
	AD9833_AmpSet(20);
	// 继电器
	Relay_Init();
	// 7606初始化
	GPIO_AD7606_Configuration();
	AD7606_Init();
	while (1)
	{	
		while(usize >= FRAMELENGTH)
		{
		  //校验帧头帧尾是否匹配
		  if(u(0) != 0x55 || u(3) != 0xff || u(4) != 0xff || u(5) != 0xff)
		  {
			  //不匹配删除1字节
			  udelete(1);
		  }else
		  {
			  //匹配，跳出循环
			  switch(u(1)){
				  // 开始测量
				  case 0x81:
						OLED_ShowNum(1,1,81,2);
						Measure_LC(100);	
					 	break;
			  }
			  break;
		  }
	  }

	  //进行解析
	  if(usize >= FRAMELENGTH && u(0) == 0x55 && u(3) == 0xff && u(4) == 0xff && u(5) == 0xff)
	  {
			udelete(FRAMELENGTH);
	  }
		//delay_ms(1000);
		//		Measure_ri();
		//		Measure_ro();
		//		Measure_a();
		// 扫频
		// vf = Change_freq();
		// Detect();
		// Delay_s(3);		
	}
}

/**
 * @brief  测量电压幅值
 * @param  adc 采样通道号
 * @param  method 计算方法，0为峰峰值，1为平均值
 * @retval v0 电压幅值
 */
float Measure_u(uint8_t adc, uint8_t method)
{
	float MaxV0, vtemp = 0.0;
	float MinV0 = 10000.0;
	i = 1;
	v = 0;
	while (i > 0)
	{
		AD7606_startconvst();
		Delay_ns(1);
		temp = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5); // 读取 BUSY的状态
		while (temp == Bit_SET)							 // 当busy为低电平时，数据转换完毕，此时可以读取数据
		{
			Delay_ns(10);
			temp = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5); // 读取 BUSY的状态
		}
		AD7606_read_data(DB_data);

		vtemp = (float)DB_data[adc] * RANGE / 32768;
		OLED_ShowSignedNum(2, 1, i, 5);
		if (MaxV0 < vtemp)
			MaxV0 = vtemp;
		if (MinV0 > vtemp)
			MinV0 = vtemp;

		Delay_ms(30);

		// 得到幅值
		if (method == 0)
			v0 = MaxV0 - MinV0;
		else
			v0 = (MaxV0 + MinV0) / 2;
		//		OLED_ShowSignedNum(1,1,vtemp, 5);
		//		OLED_ShowSignedNum(1,7,v0, 5);
		//		OLED_ShowSignedNum(3,1,MaxV0, 5);
		//		OLED_ShowSignedNum(4,1,MinV0, 5);
		// MM
		if (i == 10)
		{
			MaxV0 = 0;
			MinV0 = 10000;
		}
		if (i == 40)
		{
			MaxV0 = 0;
			MinV0 = 10000;
			i = 0;
			return v0;
		}
		i++;
		// 继电器翻转
	}
	return RANGE;
}

/**
 * @brief  扫频
 * @retval vf_t 上截止频率
 * @attention 扫频下限为1kHz，上限约为300kHz，步进值为1kHz。
 */
long Change_freq(void)
{
	uint16_t vf, vf_temp = 0;
	uint8_t k = 0;
	// 扫频从继电器的低电平开始
	Relay_reset();
	AD9833_WaveSeting(1000, 0, SIN_WAVE, 0);
	Delay_s(1);
	vf = Measure_u(2, 1);
	OLED_Clear();
	OLED_ShowString(1, 1, "vf0:");
	OLED_ShowSignedNum(1, 5, vf, 6);

	while (k < 18)
	{
		// AD9833_WaveSeting(1000-k*50,0,SIN_WAVE,0);
		AD9833_WaveSeting(10000 * k + 1000, 0, SIN_WAVE, 0);
		vf_temp = Measure_u(2, 1);
		OLED_ShowSignedNum(4, 1, vf_temp, 6);
		if (vf_temp > 0.6 * vf && vf_temp < 0.7 * vf)
		{
			// OLED_ShowSignedNum(3,4,1000-k*50,6);
			OLED_ShowSignedNum(3, 4, 10000 * k + 1000, 6);
			Delay_s(5);
			return 10000 * k + 1000;
			// return 1000-k*50;
		}
		k++;
	}
	OLED_ShowSignedNum(3, 4, 10000 * k + 1000, 6);
	return 300000;
}

long Change_wave(uint8_t kmax, int16_t step, uint16_t vf)
{
	uint8_t k = 0;
	// 采样电压值
	uint16_t vf_temp = 0;
	while (k < kmax)
	{
		AD9833_WaveSeting(1000 + k * step, 0, SIN_WAVE, 0);
		vf_temp = Measure_u(2, 1);
		OLED_ShowSignedNum(4, 1, vf_temp, 6);
		if (vf_temp > 0.6 * vf && vf_temp < 0.7 * vf)
		{
			// 可认为已截止
			OLED_ShowSignedNum(3, 4, 1000 + k * step, 6);
			Delay_s(5);
			return step * k + 1000;
		}
		k++;
	}
	return 0;
}

/**
 * @brief 测量阻抗
 * @param freq 此时的频率
 * @note  对总电压、待测、已知电阻采样，并将数据显示在串口屏上 
 * @attention  通过余弦定理求解阻抗三角形
 */
void Measure_LC(uint16_t freq)
{	
	float u1,u2,u3,x,Cx=0;
	AD9833_WaveSeting(freq, 0, SIN_WAVE, 0);
	Serial_SendString("result.z3.txt=\"正在测量\"\xff\xff\xff");
	u1 = Measure_u(0,1)/0.9; // 已知的电阻两端电压
	Print_float(8, u1, "process");
	u2 = Measure_u(2,1)/1.414*0.92/0.982; // 输入总电压-测一次
	Print_float(1, u2, "process");
	Relay_set();
	OLED_ShowNum(4,1,u1*100,7);
	OLED_ShowNum(3,1,u2*100,7);
	Delay_s(3);
	u3 = Measure_u(0,1)/0.975; // 待测网络两端电压-第二次
	OLED_ShowNum(1,1,u3*100,7);
	Print_float(7, u3, "process");
	Relay_reset();
	Delay_s(3);
	x = Calculate_Rx(u2, u3, u1, 1000);
	OLED_Clear();
	OLED_ShowNum(3,4,x*1000,7);
	Cx=Calculate_Cx(x,freq);
	OLED_ShowNum(3,4,Cx*1000,7);
	Serial_SendString("result.z3.txt=\"测量完成\"\xff\xff\xff");
}

/**
 * @brief 测量阻抗
 * @attention 使用乘法器电路实现测量
 * @param freq 当前的频率
 */
void Measure_LC2(uint16_t freq)
{
	float u1,u2,x,cos=0;
	AD9833_WaveSeting(freq, 0, SIN_WAVE, 0);
	Serial_SendString("result.z3.txt=\"正在测量\"\xff\xff\xff");
	u1 = Measure_u(0,1); // 待测两端的电压
	Print_float(8, u1, "process");
	u2 = Measure_u(2,1)/1.414*0.92; // 乘法器输出电压
	Print_float(1, u2, "process");
	cos = Calculate_cos(u1,u2);
	x = Calculate_C(cos,u1, u2, 1000);
	Serial_SendString("result.z3.txt=\"测量完成\"\xff\xff\xff");
	 
}