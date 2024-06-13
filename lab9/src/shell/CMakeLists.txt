set(SRCS shmsg.c)
add_library(shell OBJECT ${SRCS})  # OBJECT类型只编译生成.o目标文件，但不实际链接成库