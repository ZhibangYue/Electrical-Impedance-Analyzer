/**
 * @file LC.c
 * @author Zhibang Yue (yuezhibang@126.com)
 * @brief 与LC测量、计算有关的函数
 * @version 0.1
 * @date 2023-07-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "math.h"
#include "OLED.h"
#include "AD9833.h"
#include "Serial.h"
#include "Delay.h"
#include "tjc_usart_hmi.h"
#include "LC.h"

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
 * @brief 计算电感的值
 * @param x 电抗值
 * @param f 频率
 * @return Lx 电感值
 * @note  通过测算所得的电抗值计算电容值 
 */
float Calculate_Lx(float x, float f)
{
  float Lx;
  Lx = x*1000/(2*3.1415926*f);
  char str[70];
  sprintf(str, "result.l3.txt=\"%f\"\xff\xff\xff", Lx);
  Serial_SendString(str);
  return Lx;
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
  
  return x;
}


/**
 * @brief  通过乘法器电路计算余弦值
 * 
 * @param u1 待测两端的电压
 * @param u2 乘法器输出的电压
 * @return cos 余弦值
 */
float Calculate_cos(float u1, float u2)
{
  float cos;
  cos = 2*u2 / u1;
  Print_float(10, cos, "process");
  return cos;
}

/**
 * @brief 计算电抗值
 * 
 * @param cos 余弦值
 * @param u1 待测两端的电压
 * @param u2 乘法器输出的电压
 * @param R 已知电阻
 * @return x 电抗值 
 */
float Calculate_C(float cos, float u1, float u2, float R)
{
  float a, b, Rx, x;
  a = cos * u2 - u1;
  Print_float(20, a, "process");
  b = sqrt(1 - cos * cos) * u2;
  Print_float(9, sqrt(1 - cos * cos), "process");
  // Rx为测得阻抗的实部
  Rx = a * R / u2;
  Print_float(15, Rx, "result");
  // x为测得阻抗的虚部
  x = b * Rx / a;
  Print_float(22, x, "result");
  return x;
}

float Calculate_LC(float Rx, float R1, float u1)
{
  float Xc;
  Xc = R1*sqrt(U0*U0-pow(((Rx+R1)*u1/R1),2))/u1;
  return Xc;
}

/**
 * @brief 测量电抗值
 * @note 倍频半抗法
 * @param R1 
 * @param u1 
 * @param u2 
 * @return float 
 */
float Calculate_X_halfL(float R1, float u1, float u2)
{
  float X;
  X = sqrt(U0*U0*R1*R1*4*(1/(u1*u1)-(1/(u2*u2)))/3);
  return X;
}

/**
 * @brief 测量电阻值
 * @note 倍频半抗法
 * @param R1 已知电阻值
 * @param X 电抗值 
 * @param u1 测量电压
 * @return Rx 电阻值 
 */
float Calculate_Rx_halfL(float R1, float X, float u1)
{
  float Rx;
  Rx = sqrt(pow((U0*R1/u1),2)-X*X)-R1;
  return Rx;
}