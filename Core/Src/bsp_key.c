#include <bsp_key.h>

// 在文件最上面定义全局标志位，给 main.c 用
volatile uint8_t car_start_flag = 0; // 0待机，1发车
volatile uint8_t key_stop_flag = 0;  // 1表示按下了停车键

/*重写中断回调处理函数*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case key1_Pin: // KEY1 按下
		car_start_flag = 1; // 告诉主程序：可以发车了！
		step = 0;      //!!!step = = = 0
		break;
	case key2_Pin: // KEY2 按下
		car_start_flag = 0; // 告诉主程序：停车！
		step = 8;
		break;
	case key3_Pin: // KEY3 按下
		// 你可以在这里做其他测试功能，或者先留空
		break;
	}
}

