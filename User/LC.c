#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "math.h"
#include "OLED.h"
#include "AD9833.h"
#include "Serial.h"
#include "Delay.h"
#include "tjc_usart_hmi.h"

/**
 * @brief  串口屏文本显示浮点数
 * @param  index 文本框编号
 * @param  value 浮点数
 * @param  page  页码，字符串类型，由串口屏指定
 * @note   串口屏文本编辑示例：串口通信发送字符串t0.txt="123"，其中t0为文本框编号，"123"为文本框显示内容
 */
void Print_float(uint8_t index, float value, char*page)
{
  char str[70];
  sprintf(str, "%s.t%d.txt=\"%f\"\xff\xff\xff",page, index, value);
  Serial_SendString(str);
  return;
}

/**
 * @brief  计算余弦值
 * @param  a
 * @param  b
 * @param  c
 * @retval cos c边对角的余弦值
 */
float cosc(float a, float b, float c)
{
  float cos;
  cos = (a * a + b * b - c * c) / (2 * a * b);
  return cos;
}

/**
 * @brief 计算电容的值
 * 
 * @param x 电抗值
 * @param f 频率
 * @return Cx 电容值
 * @note  通过测算所得的电抗值计算电容值 
 */
float Calculate_Cx(float x, float f)
{
  float Cx;
  Cx = 1000000000/(2*3.1515926*f*x);
  char str[70];
  sprintf(str, "result.c1.txt=\"%f\"\xff\xff\xff", Cx);
  Serial_SendString(str);
  return Cx;
}

/**
 * @brief  计算测量阻抗的虚部
 * @param  u1 总电压
 * @param  u2 待测
 * @param  u3 已知
 * @param  R 已知电阻
 * @retval x 虚部
 */
float Calculate_Rx(float u1, float u2, float u3, float R)
{
  float cosu2, a, b, Rx, x, Cx;
  // cosu2为u2对角的余弦值
  cosu2 = cosc(u1, u3, u2);
  if(cosu2>1)cosu2=1;
  Print_float(10, cosu2, "process");
  OLED_ShowNum(4, 1, cosu2 * 100, 7);
  Delay_s(3);
  a = cosu2 * u1 - u3;
  Print_float(20, a * 100, "process");
  OLED_ShowNum(2, 1, a * 100, 7);
  b = sqrt(1 - cosu2 * cosu2) * u1;
  Print_float(9, sqrt(1 - cosu2 * cosu2), "process");
  // Rx为测得阻抗的实部
  Rx = a * R / u3;
  Print_float(15, Rx, "result");
  OLED_ShowNum(1, 1, Rx * 100, 7);
  Delay_s(3);
  // x为测得阻抗的虚部
  x = b * Rx / a;
  Print_float(22, x, "result");
 Cx=Calculate_Cx(x,400);
  return x;
}



void Show_x()
{
  return;
}

/**
 * @brief  判断感性/容性
 * @param  无
 * @retval 无
 */
void LC()
{
  float u0, u1, x0, x1 = 0.0;
  // 先测1000Hz时的电压
  AD9833_WaveSeting(1000, 0, SIN_WAVE, 0);
  u0 = Measure_u(2, 1);
  // 再测10000Hz时的电压
  AD9833_WaveSeting(10000, 0, SIN_WAVE, 0);
  u1 = Measure_u(2, 1);
  // 频率变大，电抗变大，则为感性
  if (x0 < x1)
    OLED_ShowString(1, 1, "Inductance");
  // 频率变大，电抗变小，则为容性
  else
    OLED_ShowString(1, 1, "Capacitance");
  return;
}
