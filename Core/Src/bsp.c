#include "main.h"
#include "bsp.h"

extern uint16_t g_car_time;
int step = 0;
static uint32_t last_time = 0;
static int speed = 800; // 报错那个，不能只写 static speed = 300;

void BSP_Init(void)
{
	Bsp_Tim_Init();
	PID_Param_Init();//电机PID初始化
	Delay_Init();
	printf("hello \r\n");
}


void BSP_Loop(void)
{
	uint32_t current_time = HAL_GetTick();


		      switch (step)
		      {
		          case 0: // 前进2秒
		              Motor_Set_Pwm(MOTOR_ID_M1, speed);
		              Motor_Set_Pwm(MOTOR_ID_M2, speed);
		              Motor_Set_Pwm(MOTOR_ID_M3, speed);
		              Motor_Set_Pwm(MOTOR_ID_M4, speed);
		              last_time = current_time;
		              step = 1;
		              printf("Step 1: Forward\r\n");
		              break;

		          case 1: // 等待2秒
		              if (current_time - last_time >= 2000)
		              {
		                  step = 2;
		                  printf("Step 2: Turn Left\r\n");
		              }
		              break;

		          case 2: // 左转 (左轮反转，右轮正转)

		              Motor_Set_Pwm(MOTOR_ID_M1, -speed); // M1反转
		              Motor_Set_Pwm(MOTOR_ID_M2, -speed); // M2反转
		              Motor_Set_Pwm(MOTOR_ID_M3, speed);  // M3正转
		              Motor_Set_Pwm(MOTOR_ID_M4, speed);  // M4正转


		              last_time = current_time;
		              step = 3;
		              break;

		          case 3: // 等待转弯1秒
		              if (current_time - last_time >=2000)
		              {
		                  step = 4;
		                  printf("Step 3: Forward Again\r\n");
		              }
		              break;

		          case 4: // 再前进2秒
		              Motor_Set_Pwm(MOTOR_ID_M1, speed);
		              Motor_Set_Pwm(MOTOR_ID_M2, speed);
		              Motor_Set_Pwm(MOTOR_ID_M3, speed);
		              Motor_Set_Pwm(MOTOR_ID_M4, speed);
		              last_time = current_time;
		              step = 5;
		              break;

		          case 5: // 等待2秒后停车
		              if (current_time - last_time >= 2000)
		              {
		                  Motor_Stop(0); // 刹车
		                  step = 6;
		                  printf("Step 4: Finished!\r\n");
		              }
		              break;

		          case 6:
		        	  Motor_Set_Pwm(MOTOR_ID_M1, -speed);
		        	  		              Motor_Set_Pwm(MOTOR_ID_M2, -speed);
		        	  		              Motor_Set_Pwm(MOTOR_ID_M3, -speed);
		        	  		              Motor_Set_Pwm(MOTOR_ID_M4, -speed);
		        	  		              last_time = current_time;
		        	  		              step = 7;
		              break;
		          case 7:
		        	  if (current_time - last_time >= 2000)
		        	  		              {
		        	  		                  Motor_Stop(0); // 刹车
		        	  		                  step = 8;
		        	  		                  printf("Step 4: Finished!\r\n");
		        	  		              }

		        	  break;

		          case 8:

		        	  break;

		      }


}
