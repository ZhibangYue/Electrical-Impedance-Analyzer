/**
 * @file LC.h
 * @author Zhibang Yue (yuezhibang@126.com)
 * @brief LC的测量与计算
 * @version 0.1
 * @date 2023-07-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __LC_H
#define __LC_H

#define KNOWN_R 10000    //已知电阻的阻值
#define U0 1736         //电压初值

float cosc(float a, float b, float c);
void Print_float(uint8_t index, float value, char*page);
float Calculate_Rx(float u1, float u2, float u3, float R);
float Calculate_Cx(float x, float f);
float Calculate_Lx(float x, float f);
float Calculate_cos(float u1, float u2);
float Calculate_C(float cos, float u1, float u2, float R);

float Calculate_LC(float Rx, float R1, float u1);
float Calculate_X_halfL(float R1, float u1, float u2);
float Calculate_Rx_halfL(float R1, float X, float u1);
#endif
