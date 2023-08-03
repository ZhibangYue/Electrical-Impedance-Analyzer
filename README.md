# Electrical-Impedance-Analyzer
C program to measure electrical impedance by stm32f407vet6

**2023年电子设计大赛模拟《阻抗测量分析仪》**

## v0.0初版(2023/7/30)

硬件Hardware中：

- AD9833输出正弦信号
- AD7606实现电压采样
- OLED和淘晶驰串口屏实现数据显示与人机交互
- relay为继电器模块
- key为按键

User中：

- LC收纳与阻抗计算相关的函数等

初版通过对阻抗三角形应用余弦定理计算**阻抗角**，而后由此测算各参量，并通过频率变化判定感性与容性。

## v1.0更新(2023/7/31)
1.0版增加了通过乘法器电路进行对电抗的计算（简略版）。
与0初版相比两者都存在相当的局限性，虽然也许是硬件电路的问题。

## v1.1更新(2023/8/1)
1.1版更新了一种测量阻抗的新方法，实际上以上各方法都有各自的适用区间，但目前还无法将其整合。  
同时增加了**波形发生器**的控制函数`Show()`，装置具备了通过串口屏控制AD9833发送信号的能力。