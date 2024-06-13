set(SRCS prt_sys.c prt_task_init.c prt_task.c)
add_library(task OBJECT ${SRCS})  # OBJECT类型只编译生成.o目标文件，但不实际链接成库
