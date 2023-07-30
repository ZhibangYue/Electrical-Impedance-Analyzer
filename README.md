# Electrical-Impedance-Analyzer
C program to measure electrical impedance by stm32f407vet6

**2023年电子设计大赛模拟《阻抗测量分析仪》**

## v0.0初版

硬件Hardware中：

- AD9833输出正弦信号
- AD7606实现电压采样
- OLED和淘晶驰串口屏实现数据显示与人机交互
- relay为继电器模块
- key为按键

User中：

- LC收纳与阻抗计算相关的函数等