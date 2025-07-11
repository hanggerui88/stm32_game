#include "stm32f10x.h"                  // Device header
void timerini()
{
	TIM_TimeBaseInitTypeDef timer_ini;
	NVIC_InitTypeDef nvic_ini_a;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_InternalClockConfig(TIM2);
	timer_ini.TIM_ClockDivision=TIM_CKD_DIV1;
	timer_ini.TIM_CounterMode=TIM_CounterMode_Up;
	timer_ini.TIM_Period=10000-1;//0.125s
	timer_ini.TIM_Prescaler=900-1;
	timer_ini.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&timer_ini);
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	nvic_ini_a.NVIC_IRQChannel=TIM2_IRQn;
	nvic_ini_a.NVIC_IRQChannelCmd=ENABLE;
	nvic_ini_a.NVIC_IRQChannelPreemptionPriority=1;
	nvic_ini_a.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&nvic_ini_a);
	TIM_Cmd(TIM2,ENABLE);
}
