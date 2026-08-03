# 服务器源文件：明确列出所有源文件
set(SERVER_SOURCES
    ${PROJECT_SOURCE_DIR}/server/server.cpp
    ${PROJECT_SOURCE_DIR}/server/handle.cpp
    ${PROJECT_SOURCE_DIR}/server/Reactor.cpp
    ${PROJECT_SOURCE_DIR}/server/sendrecv.cpp
    ${PROJECT_SOURCE_DIR}/server/userdata.cpp
    ${PROJECT_SOURCE_DIR}/socket/socket.cpp
    ${PROJECT_SOURCE_DIR}/message.cpp
    ${PROJECT_SOURCE_DIR}/group.cpp
    ${PROJECT_SOURCE_DIR}/user.cpp
    ${PROJECT_SOURCE_DIR}/report.cpp
    ${PROJECT_SOURCE_DIR}/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/Redis.cpp
    ${PROJECT_SOURCE_DIR}/client/register/hashpwd.cpp
)

add_executable(server_app ${SERVER_SOURCES})

# 设置包含目录
target_include_directories(server_app PRIVATE
    ${PROJECT_SOURCE_DIR}/server
    ${PROJECT_SOURCE_DIR}/socket
    ${PROJECT_SOURCE_DIR}
)

# 链接必要的库
target_link_libraries(server_app pthread hiredis sodium curl glog)

# 设置 C++ sanitize 标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
