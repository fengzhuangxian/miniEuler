# sh runMiniEuler.sh 直接运行
# sh runMiniEuler.sh -S 启动后在入口处暂停等待调试

echo qemu-system-aarch64 -machine virt,gic-version=2 -m 1024M -cpu cortex-a53 -nographic -kernel build/miniEuler  -s $1

qemu-system-aarch64 -machine virt,gic-version=2 -m 1024M -cpu cortex-a53 -nographic -kernel build/miniEuler  -s $1
