#include "stm32f4xx.h"                  // Device header


void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	// RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_DeInit();//ADC复位
	
	// 统筹设置ADC
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	// 设置为单ADC模式
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	// 两次采样点之间的延迟
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	// 设置DMA模式为关闭
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	// 设置ADC分频，这里使用了六分频
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
	ADC_CommonInit(&ADC_CommonInitStructure);

	//设置ADC1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_56Cycles);
	ADC_InitTypeDef ADC_InitStructure;
	// 右对齐
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	// 开启非扫描模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	// 开启单次模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	// 选择不使用外部触发
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	// 选择ADC转换的通道
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	// 使能ADC1
	ADC_Cmd(ADC1, ENABLE);
	
}

uint16_t AD_GetValue(void)
{	
	// 启动ADC1对应通道的转换
	ADC_SoftwareStartConv(ADC1);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}
