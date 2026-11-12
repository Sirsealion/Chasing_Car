#ifndef __FOLLOW_H_
#define __FOLLOW_H_

#include "main.h"

void Follow_Init(void);
void Follow_Update(void);      /* 主循环里反复调用 */
void Follow_Stop(void);
void Line_Update(void);

#endif
