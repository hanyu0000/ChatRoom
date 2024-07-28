cmake_minimum_required(VERSION 3.10)
project(ChatApp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 查找 Hiredis 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)
# 包含 Hiredis 头文件
include_directories(${HIREDIS_INCLUDE_DIRS})
# 包含项目头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
# 添加源文件
set(SOURCES
    src/client/client.cpp
    src/client/gui.cpp
    src/server/server.cpp
    src/server/t_main.cpp
)
# 添加客户端可执行文件
add_executable(client src/client/client.cpp src/client/tui.cpp)
target_link_libraries(client ${HIREDIS_LIBRARIES})
# 添加服务器可执行文件
add_executable(server src/server/server.cpp src/server/t_main.cpp)
target_link_libraries(server ${HIREDIS_LIBRARIES})
# 安装规则
install(TARGETS client server DESTINATION bin)