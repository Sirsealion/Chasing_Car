#include "main.h"
#include "bsp_usart.h"

#define USART_DEBUG   huart1

/* ================= 环形缓冲 ================= */
#define RB_SIZE 256

typedef struct {
    uint8_t  buf[RB_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} ring_t;

static ring_t rb_pc;      /* 电脑 -> 车 */
static ring_t rb_k210;    /* K210 -> 车 */

volatile int g_t_w = 0;      /* 目标宽度（像素） */

static uint8_t RxTemp  = 0;   /* USART1 */
static uint8_t RxTemp2 = 0;   /* UART4  */

/* 电脑 -> K210 的行缓冲 */
static uint8_t  pc_line[64];
static uint16_t pc_len = 0;

/* ================= K210 目标信息 ================= */
volatile int      g_t_dx = 0;
volatile int      g_t_dy = 0;
volatile uint8_t  g_t_found = 0;
volatile uint32_t g_t_tick = 0;

volatile uint32_t g_k210_cnt = 0;    /* 收到的字节数 */
volatile uint32_t g_t_frames = 0;    /* 解析成功的帧数 */

static uint8_t  k_line[32];
static uint16_t k_len = 0;

static int parse_int(const uint8_t **pp)
{
    int v = 0, sign = 1;
    const uint8_t *p = *pp;

    if (*p == '-') { sign = -1; p++; }
    while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; }

    *pp = p;
    return v * sign;
}

void K210_ParseByte(uint8_t b)
{
    g_k210_cnt++;

    if (b == '\r') return;

    if (b == '\n')
    {
        k_line[k_len] = 0;

        if (k_len > 4 && k_line[0] == '$' && k_line[1] == 'T' && k_line[2] == ',')
        {
            const uint8_t *p = k_line + 3;

            int dx = parse_int(&p);          /* 第 1 个：dx */

            if (*p == ',')
            {
                p++;
                int dy = parse_int(&p);      /* 第 2 个：dy */

                if (*p == ',')
                {
                    p++;
                    int fd = parse_int(&p);  /* 第 3 个：found */

                    if (*p == ',')
                    {
                        p++;
                        int w = parse_int(&p);   /* 第 4 个：宽度 */

                        g_t_dx    = dx;
                        g_t_dy    = dy;
                        g_t_found = (uint8_t)(fd ? 1 : 0);
                        g_t_w     = w;
                        g_t_tick  = HAL_GetTick();
                        g_t_frames++;
                    }
                }
            }
        }
        k_len = 0;
    }
    else if (k_len < sizeof(k_line) - 1)
    {
        k_line[k_len++] = b;
    }
}

void K210_Update(void)
{
    if (HAL_GetTick() - g_t_tick > 500)
    {
        g_t_found = 0;
    }
}

static int rb_put(ring_t *r, uint8_t b)
{
    uint16_t next = (uint16_t)((r->head + 1) % RB_SIZE);
    if (next == r->tail) return 0;          /* 满了就丢 */
    r->buf[r->head] = b;
    r->head = next;
    return 1;
}

static int rb_get(ring_t *r, uint8_t *b)
{
    if (r->head == r->tail) return 0;
    *b = r->buf[r->tail];
    r->tail = (uint16_t)((r->tail + 1) % RB_SIZE);
    return 1;
}

/* ================= printf ================= */
#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&USART_DEBUG, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

/* ================= 初始化 ================= */
void USART1_UART_Init(void)
{
  HAL_UART_Receive_IT(&huart1, &RxTemp, 1);
}

void USART4_UART_Init(void)
{
  HAL_UART_Receive_IT(&huart4, &RxTemp2, 1);
}

/* ================= 发送 ================= */
void USART1_DataByte(uint8_t b)                { HAL_UART_Transmit(&huart1, &b, 1, 100); }
void USART1_DataString(uint8_t *s, uint16_t n) { HAL_UART_Transmit(&huart1, s, n, 100); }
void K210_DataByte(uint8_t b)                  { HAL_UART_Transmit(&huart4, &b, 1, 100); }
void K210_DataString(uint8_t *s, uint16_t n)   { HAL_UART_Transmit(&huart4, s, n, 100); }

/* ============ 中断：只入队，不发送 ============ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    HAL_UART_Receive_IT(&huart1, &RxTemp, 1);
    rb_put(&rb_pc, RxTemp);
  }
  else if (huart->Instance == UART4)
  {
    HAL_UART_Receive_IT(&huart4, &RxTemp2, 1);
    rb_put(&rb_k210, RxTemp2);
  }
}

/* ============ 主循环里调用：统一发送 ============ */
void USART_Process(void)
{
  uint8_t b;
  uint8_t crlf[2] = {'\r', '\n'};

  /* K210 -> 电脑 */
  while (rb_get(&rb_k210, &b))
  {
    //HAL_UART_Transmit(&huart1, &b, 1, 10);
    K210_ParseByte(b);
  }

  /* 电脑 -> K210（@ 开头的行）+ 回显 */
  while (rb_get(&rb_pc, &b))
  {
    HAL_UART_Transmit(&huart1, &b, 1, 10);    /* 回显，不想要就删 */

    if (b == '\r') continue;

    if (b == '\n')
    {
      if (pc_len > 0 && pc_line[0] == '@')
      {
        K210_DataString(pc_line + 1, pc_len - 1);
        K210_DataString(crlf, 2);
      }
      pc_len = 0;
    }
    else if (pc_len < sizeof(pc_line) - 1)
    {
      pc_line[pc_len++] = b;
    }
  }
}
