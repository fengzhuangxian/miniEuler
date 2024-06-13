set(SRCS start.S prt_reset_vector.S print.c vsnprintf_s.c os_exc_armv8.h prt_exc.c prt_vector.S hwi_init.c timer.c os_attr_armv8_external.h os_cpu_armv8_external.h mmu.c mmu.h cache_asm.S)
add_library(bsp OBJECT ${SRCS})  # OBJECT类型只编译生成.o目标文件，但不实际链接成库
