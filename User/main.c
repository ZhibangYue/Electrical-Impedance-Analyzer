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
#include "math.h"

#define FRAMELENGTH 6

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
void Detect(void);
void Measure_LC(void);
float cosc(float a, float b, float c);
int main(void)
{	
	char str[40];
	float u1,u2,u3,x=0.0;
	OLED_Init();
	Serial_Init();
	// AD9833发波
	AD9833_Init_GPIO();
	AD9833_WaveSeting(400.0, 0, SIN_WAVE, 0);
	AD9833_AmpSet(20);
	// 继电器
	Relay_Init();
	// 7606初始化
	GPIO_AD7606_Configuration();
	AD7606_Init();
	while (1)
	{	
		OLED_ShowNum(4,1,usize,5);
		while(usize >= FRAMELENGTH)
		{
		OLED_ShowNum(4,1,usize,5);
		  //校验帧头帧尾是否匹配
		  if(u(0) != 0x55 || u(3) != 0xff || u(4) != 0xff || u(5) != 0xff)
		  {
			  //不匹配删除1字节
			  udelete(1);
			  OLED_ShowHexNum(3,1,u(1),6);
		  }else
		  {
			  //匹配，跳出循环
			  switch(u(1)){
				  // 开始测量
				  case 0x81:
						OLED_ShowNum(1,1,81,2);
						Measure_LC();	
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

		vtemp = (float)DB_data[adc] * 5000.0 / 32768;
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
}

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

void Show_details(void)
{
	OLED_ShowString(1, 1, "ri:");
	OLED_ShowSignedNum(1, 4, ri, 5);
	OLED_ShowString(2, 1, "a:");
	OLED_ShowSignedNum(2, 4, a, 5);
	OLED_ShowString(3, 1, "vf:");
	OLED_ShowNum(3, 4, vf, 7);
}


/**
 * @brief 测量阻抗
 * 
 * @note  对总电压、待测、已知电阻采样，并将数据显示在串口屏上 
 */
void Measure_LC(void)
{	
	float u1,u2,u3,x=0;
	Serial_SendString("result.z3.txt=\"正在测量\"\xff\xff\xff");
	u1 = Measure_u(0,1)/0.9; // 已知的电阻两端电压
	sprintf(str,"process.t8.txt=\"%f\"\xff\xff\xff",u1);
	Serial_SendString(str);
	u2 = Measure_u(2,1)/1.414*0.92/0.982; //输入总电压-测一次
	sprintf(str,"process.t1.txt=\"%f\"\xff\xff\xff",u2);
	Serial_SendString(str);
	Relay_set();
	OLED_ShowNum(4,1,u1*100,7);
	OLED_ShowNum(3,1,u2*100,7);
	Delay_s(3);
	u3 = Measure_u(0,1)/0.975; // 待测网络两端电压-第二次
	OLED_ShowNum(1,1,u3*100,7);
	sprintf(str,"process.t7.txt=\"%f\"\xff\xff\xff",u3);
	Serial_SendString(str);
	Relay_reset();
	Delay_s(3);
	x = Calculate_Rx(u2, u3, u1, 1000);
	OLED_Clear();
	OLED_ShowNum(3,4,x*1000,7);
	Serial_SendString("result.z3.txt=\"测量完成\"\xff\xff\xff");
}