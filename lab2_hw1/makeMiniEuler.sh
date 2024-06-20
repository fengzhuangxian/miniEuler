# sh makeMiniEuler.sh 不打印编译命令
# sh makeMiniEuler.sh -v 打印编译命令等详细信息
rm -rf build/*
mkdir build
cd build
cmake ../src
cmake --build . $1