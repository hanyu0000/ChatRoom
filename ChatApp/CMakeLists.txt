cmake_minimum_required(VERSION 3.10)
project(ChatApp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# 查找 Hiredis 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)
include_directories(${HIREDIS_INCLUDE_DIRS})

# 查找 ncurses 库
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

# 包含项目头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加客户端源文件
set(CLIENT_SOURCES
    src/client/client.cpp
    src/client/HHH.cpp
    src/client/manage.cpp
    src/client/TUI.cpp
)

# 添加服务器源文件
set(SERVER_SOURCES
    src/server/server.cpp
    src/server/RedisServer.cpp
    src/server/task.cpp
    src/server/ThreadPool.cpp
)

# 添加客户端可执行文件
add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client ${HIREDIS_LIBRARIES} ${CURSES_LIBRARIES})

# 添加服务器可执行文件
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server ${HIREDIS_LIBRARIES})

# 安装规则
install(TARGETS client server DESTINATION bin)
