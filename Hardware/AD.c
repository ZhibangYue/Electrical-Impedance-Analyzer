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
	
	ADC_DeInit();//ADC��λ
	
	// ͳ������ADC
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	// ����Ϊ��ADCģʽ
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	// ���β�����֮����ӳ�
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	// ����DMAģʽΪ�ر�
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	// ����ADC��Ƶ������ʹ��������Ƶ
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
	ADC_CommonInit(&ADC_CommonInitStructure);

	//����ADC1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_56Cycles);
	ADC_InitTypeDef ADC_InitStructure;
	// �Ҷ���
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	// ������ɨ��ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	// ��������ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	// ѡ��ʹ���ⲿ����
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	// ѡ��ADCת����ͨ��
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	// ʹ��ADC1
	ADC_Cmd(ADC1, ENABLE);
	
}

uint16_t AD_GetValue(void)
{	
	// ����ADC1��Ӧͨ����ת��
	ADC_SoftwareStartConv(ADC1);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}
