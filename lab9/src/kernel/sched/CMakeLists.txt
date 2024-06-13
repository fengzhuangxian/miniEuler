set(SRCS prt_sched_single.c)
add_library(sched OBJECT ${SRCS})  # OBJECT类型只编译生成.o目标文件，但不实际链接成库
