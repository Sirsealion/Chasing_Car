#include "follow.h"
#include "app_motor.h"
#include "bsp_usart.h"

/* ================= 参数 ================= */
#define TURN_K        4.0f
#define FILTER        0.25f

#define VZ_MAX        300
#define DEAD_DX       4
#define DX_JUMP_MAX   25
#define VZ_SLEW       25

#define W_TARGET      25      /* 期望的物体宽度（像素）—— 要实测！ */
#define W_BAND        10      /* ±12 像素内认为距离合适，不动 */
#define VX_KP         1.0f     /* 宽度误差 -> 速度 */
#define VX_MAX        100
#define VX_SLEW       20       /* 每 10ms 最多变化 */

#define LOST_MS       300

/* ================= 状态 ================= */
static float s_dx          = 0.0f;
static int   s_dx_last_raw = 0;
static int   s_vz          = 0;
static int   s_vx          = 0;

/* ================= 巡线参数 ================= */
#define LINE_BASE_SPEED   100  /* 基础速度 mm/s，先慢 */
#define LINE_TURN_K       3.5f    /* 误差 -> 转向 */
#define LINE_VZ_MAX       400
#define LINE_LOST_MS      300

void Line_Update(void)
{
    /* 丢线 / 超时 → 停车 */
    if (!g_t_found || (HAL_GetTick() - g_t_tick > LINE_LOST_MS))
    {
        Motion_Stop(STOP_BRAKE);
        return;
    }

    /* 转向 */
    int vz = (int)(LINE_TURN_K * g_t_dx);

    if (vz >  LINE_VZ_MAX) vz =  LINE_VZ_MAX;
    if (vz < -LINE_VZ_MAX) vz = -LINE_VZ_MAX;

    /* 前进速度固定，转向交给 vz */
    Motion_Ctrl(LINE_BASE_SPEED, 0, (int16_t)vz);
}


void Follow_Init(void)
{
    s_dx = 0.0f; s_dx_last_raw = 0; s_vz = 0; s_vx = 0;
}

void Follow_Stop(void)
{
    Motion_Stop(STOP_BRAKE);
    s_dx = 0.0f; s_dx_last_raw = 0; s_vz = 0; s_vx = 0;
}

void Follow_Update(void)
{
    if (!g_t_found || (HAL_GetTick() - g_t_tick > LOST_MS))
    {
        Follow_Stop();
        return;
    }

    /* ---- 跳变保护 ---- */
    if (s_dx_last_raw != 0)
    {
        int d = g_t_dx - s_dx_last_raw;
        if (d > DX_JUMP_MAX || d < -DX_JUMP_MAX)
        {
            s_dx_last_raw = g_t_dx;
            return;
        }
    }
    s_dx_last_raw = g_t_dx;

    /* ---- 转向 ---- */
    s_dx = s_dx * (1.0f - FILTER) + (float)g_t_dx * FILTER;
    if (s_dx > -DEAD_DX && s_dx < DEAD_DX) s_dx = 0.0f;

    int vz = (int)(TURN_K * s_dx);
    if (vz >  VZ_MAX) vz =  VZ_MAX;
    if (vz < -VZ_MAX) vz = -VZ_MAX;

    if (vz - s_vz >  VZ_SLEW) vz = s_vz + VZ_SLEW;
    if (vz - s_vz < -VZ_SLEW) vz = s_vz - VZ_SLEW;
    s_vz = vz;

    /* ---- 前后：用宽度当距离 ---- */
    int e = W_TARGET - g_t_w;          /* 正 = 太远 */

    int vx = 0;
    if (e > W_BAND)        vx = (int)(VX_KP * (e - W_BAND));   /* 太远 → 前进 */
    else if (e < -W_BAND)  vx = (int)(VX_KP * (e + W_BAND));   /* 太近 → 后退 */

    if (vx >  VX_MAX) vx =  VX_MAX;
    if (vx < -VX_MAX) vx = -VX_MAX;

    if (vx - s_vx >  VX_SLEW) vx = s_vx + VX_SLEW;
    if (vx - s_vx < -VX_SLEW) vx = s_vx - VX_SLEW;
    s_vx = vx;

    Motion_Ctrl((int16_t)vx, 0, (int16_t)vz);
}
