/*
 * bsp_tim.c
 *
 *  Created on: 2023年10月18日
 *      Author: YB-101
 */

#include "bsp_tim.h"

extern uint16_t g_car_time;//小车行驶的时间 The driving time of the car
/*
 * 初始化定时器123458 Initialize TIM1.2.3.4.5.8
 * */
void Bsp_Tim_Init(void)

{
	// 启动tim1的pwm输出 Start the pwm output of tim1
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	__HAL_TIM_MOE_ENABLE(&htim1);

	// 启动tim8的pwm输出 Start the pwm output of tim8
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
	__HAL_TIM_MOE_ENABLE(&htim8);

	TIM2->CNT = 0x7fff;
	// 启动tim2的编码器模式 Start the encoder mode of tim2
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2);

	TIM3->CNT = 0x7fff;
	// 启动tim3的编码器模式 Start the encoder mode of tim3
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1 | TIM_CHANNEL_2);

	TIM4->CNT = 0x7fff;
	// 启动tim4的编码器模式 Start the encoder mode of tim4
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);

	TIM5->CNT = 0x7fff;
	// 启动tim5的编码器模式 Start the encoder mode of tim5
	HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_1 | TIM_CHANNEL_2);


	//启动定时6中断
	//HAL_TIM_Base_Start_IT(&htim6);
}






//基本定时器中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)//10ms
	{
		Encoder_Update_Count();//10ms测速
		Motion_Handle();//调用PID控制速度

	}
    if(g_car_time>0)
    {
    	g_car_time--;
    }

}



