#ifndef __BSP_USART_H_
#define __BSP_USART_H_

#include "bsp.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart4;

extern volatile int      g_t_dx;
extern volatile int      g_t_dy;
extern volatile uint8_t  g_t_found;
extern volatile uint32_t g_t_tick;
extern volatile uint32_t g_k210_cnt;
extern volatile uint32_t g_t_frames;
extern volatile int g_t_w;

void K210_ParseByte(uint8_t b);
void K210_Update(void);        /* 超时检测，主循环调用 */

void USART1_UART_Init(void);
void USART4_UART_Init(void);

void USART1_DataByte(uint8_t data_byte);
void USART1_DataString(uint8_t *data_str, uint16_t datasize);

void K210_DataByte(uint8_t data_byte);
void K210_DataString(uint8_t *data_str, uint16_t datasize);

void USART_Process(void);          /* 主循环调用 */

#endif
