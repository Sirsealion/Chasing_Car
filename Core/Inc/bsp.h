#ifndef __BSP_H_
#define __BSP_H_


#include "stdio.h"
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"

#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "bsp_tim.h"
#include "bsp_PID_motor.h"
#include "app_motor.h"
#include "delay.h"
#include "bsp_usart.h"

extern volatile uint8_t car_start_flag;

void BSP_Init(void);
void BSP_Loop(void);



#endif
