cmake_minimum_required(VERSION 3.12)

set(CMAKE_SYSTEM_NAME "Generic") # 目标系统(baremental)：  cmake/tool_chain/uniproton_tool_chain_gcc_arm64.cmake 写的是Linux
set(CMAKE_SYSTEM_PROCESSOR "aarch64") # 目标系统CPU

set(TOOLCHAIN_PATH "/home/fengzhua/aarch64-none-elf") # 修改为交叉工具链实际所在目录 build.py config.xml中定义
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}/bin/aarch64-none-elf-ld)

# 定义编译和链接等选项
set(CC_OPTION "-Os   -Wformat-signedness    -Wl,--build-id=none   -fno-PIE -fno-PIE --specs=nosys.specs -fno-builtin -fno-dwarf2-cfi-asm -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -finline-limit=20  -mlittle-endian -nostartfiles -funwind-tables")
set(AS_OPTION "-Os   -Wformat-signedness    -Wl,--build-id=none   -fno-PIE -fno-PIE --specs=nosys.specs -fno-builtin -fno-dwarf2-cfi-asm -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -finline-limit=20  -mlittle-endian -nostartfiles -funwind-tables")
set(LD_OPTION " ")
set(CMAKE_C_FLAGS "${CC_OPTION} ")
set(CMAKE_ASM_FLAGS "${AS_OPTION} ")
set(CMAKE_LINK_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/aarch64-qemu.ld") # 指定链接脚本
set(CMAKE_EXE_LINKER_FLAGS "${LD_OPTION} -T ${CMAKE_CURRENT_SOURCE_DIR}/aarch64-qemu.ld") # 指定链接脚本
set (CMAKE_C_LINK_FLAGS " ")
set (CMAKE_CXX_LINK_FLAGS " ")

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(APP "miniEuler") # APP变量，后面会用到 ${APP}
project(${APP} LANGUAGES C ASM) # 工程名及所用语言
set(CMAKE_BUILD_TYPE Debug) # 生成 Debug 版本

include_directories( # include 目录
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
)

add_subdirectory(bsp) # 包含子文件夹的内容
add_subdirectory(kernel) 

list(APPEND OBJS $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:tick> $<TARGET_OBJECTS:task> $<TARGET_OBJECTS:sched>) 
add_executable(${APP} main.c ${OBJS}) # 可执行文件
