#include <stdarg.h>
#include "prt_typedef.h"
#include "prt_sem.h"
#include "prt_shell.h"

#define UART_0_REG_BASE 0x09000000 // pl011 设备寄存器地址
// 寄存器及其位定义参见：https://developer.arm.com/documentation/ddi0183/g/programmers-model/summary-of-registers
#define DW_UART_THR 0x00 // UARTDR(Data Register) 寄存器
#define DW_UART_FR 0x18  // UARTFR(Flag Register) 寄存器
#define DW_UART_LCR_HR 0x2c  // UARTLCR_H(Line Control Register) 寄存器
#define DW_XFIFO_NOT_FULL 0x020  // 发送缓冲区满置位
#define DW_FIFO_ENABLE 0x10 // 启用发送和接收FIFO

#define UART_BUSY_TIMEOUT   1000000
#define OS_MAX_SHOW_LEN 0x200

#define UARTCR_UARTEN (1 << 0)
#define UARTCR_TXE (1 << 8)
#define UARTCR_RXE (1 << 9)

#define UARTICR_ALL (1 << 0)

#define UARTIMSC_RXIM (1 << 4)

#define UARTIBRD_IBRD_MASK 0xFFFF
#define UARTFBRD_FBRD_MASK 0x3F

#define UARTLCR_H_WLEN_MASK (3 << 5)
#define UARTLCR_H_PEN (1 << 1)
#define UARTLCR_H_STP1 (0 << 3)


#define UART_REG_READ(addr)          (*(volatile U32 *)(((uintptr_t)addr)))  // 读设备寄存器
#define UART_REG_WRITE(value, addr)  (*(volatile U32 *)((uintptr_t)addr) = (U32)value) // 写设备寄存器

SemHandle sem_uart_rx;

extern void OsGicIntSetConfig(uint32_t interrupt, uint32_t config);
extern void OsGicIntSetPriority(uint32_t interrupt, uint32_t priority);
extern void OsGicEnableInt(U32 intId);
extern void OsGicClearInt(uint32_t interrupt);
extern U32 PRT_Printf(const char *format, ...);

U32 PRT_UartInit(void)
{
    U32 result = 0;
    U32 reg_base = UART_0_REG_BASE;

    UART_REG_WRITE(0, (unsigned long)(reg_base + 0x30));// 禁用pl011
    UART_REG_WRITE(0x7ff, (unsigned long)(reg_base + 0x44));// 清空中断状态
    UART_REG_WRITE(UARTIMSC_RXIM, (unsigned long)(reg_base + 0x38));// 设定中断mask，需要使能的中断
    UART_REG_WRITE(13, (unsigned long)(reg_base + 0x24));
    UART_REG_WRITE(1, (unsigned long)(reg_base + 0x28));

    // https://developer.arm.com/documentation/ddi0183/g/programmers-model/register-descriptions/line-control-register--uartlcr-h?lang=en
    result = UART_REG_READ((unsigned long)(reg_base + DW_UART_LCR_HR));
    result = result | UARTLCR_H_WLEN_MASK | UARTLCR_H_PEN | UARTLCR_H_STP1 | DW_FIFO_ENABLE;
    UART_REG_WRITE(result, (unsigned long)(reg_base + DW_UART_LCR_HR)); // 8N1 FIFO enable

    UART_REG_WRITE(UARTCR_UARTEN | UARTCR_RXE | UARTCR_TXE, (unsigned long)(reg_base + 0x30));// 启用pl011


    // 启用UART 接收中断
    OsGicIntSetConfig(33, 0); //可省略
    OsGicIntSetPriority(33, 0);
    OsGicClearInt(33); //可省略
    OsGicEnableInt(33);

    // 创建uart数据接收信号量
    U32 ret;
    ret = PRT_SemCreate(0, &sem_uart_rx);
    if (ret != OS_OK) {
        PRT_Printf("failed to create uart_rx sem\n");
        return 1;
    }

    return OS_OK;
}

// 读 reg_base + offset 寄存器的值。 uartno 参数未使用
S32 uart_reg_read(S32 uartno, U32 offset, U32 *val)
{
    S32 ret;
    U32 reg_base = UART_0_REG_BASE;


    *val = UART_REG_READ((unsigned long)(reg_base + offset));
    return OS_OK;
}

// 通过检查 FR 寄存器的标志位确定发送缓冲是否满，满时返回1.
S32 uart_is_txfifo_full(S32 uartno)
{
    S32 ret;
    U32 usr = 0;

    ret = uart_reg_read(uartno, DW_UART_FR, &usr);
    if (ret) {
        return OS_OK;
    }

    return (usr & DW_XFIFO_NOT_FULL);
}

// 往 reg_base + offset 寄存器中写入值 val。
void uart_reg_write(S32 uartno, U32 offset, U32 val)
{
    S32 ret;
    U32 reg_base = UART_0_REG_BASE;

    UART_REG_WRITE(val, (unsigned long)(reg_base + offset));
    return;
}

// 通过轮询的方式发送字符到串口
void uart_poll_send(unsigned char ch)
{

    S32 timeout = 0;
    S32 max_timeout = UART_BUSY_TIMEOUT;

    // 轮询发送缓冲区是否满
    int result = uart_is_txfifo_full(0);
    while (result) {
        timeout++;
        if (timeout >= max_timeout) {
            return;
        }
        result = uart_is_txfifo_full(0);
    }

    // 如果缓冲区没满，通过往数据寄存器写入数据发送字符到串口
    uart_reg_write(0, DW_UART_THR, (U32)(U8)ch);
    return;
}

// 轮询的方式发送字符到串口，且转义换行符
void TryPutc(unsigned char ch)
{
    uart_poll_send(ch);
    if (ch == '\n') {
        uart_poll_send('\r');
    }
}

extern int  vsnprintf_s(char *buff, int buff_size, int count, char const *fmt, va_list arg);
int TryPrintf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};
    char *str = buff;

    len = vsnprintf_s(buff, OS_MAX_SHOW_LEN, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }

    while (*str != '\0') {
        TryPutc(*str);
        str++;
    }

    return OS_OK;
}

U32 PRT_Printf(const char *format, ...)
{
    va_list vaList;
    S32 count;

    va_start(vaList, format);
    count = TryPrintf(format, vaList);
    va_end(vaList);

    return count;
}

extern ShellCB g_shellCB;
void OsUartRxHandle(void)
{
    U32 flag = 0;
    U32 result = 0;
    U32 reg_base = UART_0_REG_BASE;

    flag = UART_REG_READ((unsigned long)(reg_base + 0x18));
    while((flag & (1<<4)) == 0)
    {
        result = UART_REG_READ((unsigned long)(reg_base + 0x0));
        // PRT_Printf("%c", result);

        // 将收到的字符存到g_shellCB的缓冲区
        g_shellCB.shellBuf[g_shellCB.shellBufOffset] = (char) result;
        g_shellCB.shellBufOffset++;
        if (g_shellCB.shellBufOffset == SHELL_SHOW_MAX_LEN)
            g_shellCB.shellBufOffset = 0;

        PRT_SemPost(sem_uart_rx);
        flag = UART_REG_READ((unsigned long)(reg_base + 0x18));
    }
    return;
}