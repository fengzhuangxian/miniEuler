#include"prt_typedef.h"

#define UART_REG_WRITE(value, addr)  (*(volatile U32 *)((uintptr_t)addr) = (U32)value)

S32 main(void)
{
    char out_str[] = "AArch64 Bare Metal";

    int length = sizeof(out_str) / sizeof(out_str[0]);

    // 逐个输出字符
    for (int i = 0; i < length - 1; i++) {
        UART_REG_WRITE(out_str[i], 0x9000000);
    }
}