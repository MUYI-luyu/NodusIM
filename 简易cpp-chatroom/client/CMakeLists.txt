# 客户端源文件：明确列出所有源文件
set(CLIENT_SOURCES
    ${PROJECT_SOURCE_DIR}/client/client.cpp
    ${PROJECT_SOURCE_DIR}/client/menu.cpp
    ${PROJECT_SOURCE_DIR}/client/login/allfucs/setupfuc.cpp
    ${PROJECT_SOURCE_DIR}/client/login/allfucs/filefuc.cpp
    ${PROJECT_SOURCE_DIR}/client/login/allfucs/friendfucs.cpp
    ${PROJECT_SOURCE_DIR}/client/login/allfucs/reportfuc.cpp
    ${PROJECT_SOURCE_DIR}/client/login/allfucs/groupfucs.cpp
    ${PROJECT_SOURCE_DIR}/client/login/userfuc.cpp
    ${PROJECT_SOURCE_DIR}/client/login/MessageQueue.cpp
    ${PROJECT_SOURCE_DIR}/client/login/login.cpp
    ${PROJECT_SOURCE_DIR}/client/register/SendEmail.cpp
    ${PROJECT_SOURCE_DIR}/client/register/hashpwd.cpp
    ${PROJECT_SOURCE_DIR}/client/register/register.cpp
    ${PROJECT_SOURCE_DIR}/client/register/readdata.cpp
    ${PROJECT_SOURCE_DIR}/socket/socket.cpp
    ${PROJECT_SOURCE_DIR}/message.cpp
    ${PROJECT_SOURCE_DIR}/group.cpp
    ${PROJECT_SOURCE_DIR}/user.cpp
    ${PROJECT_SOURCE_DIR}/report.cpp

)

add_executable(client_app ${CLIENT_SOURCES})

# 设置包含目录
target_include_directories(client_app PRIVATE
    ${PROJECT_SOURCE_DIR}/client
    ${PROJECT_SOURCE_DIR}/socket
    ${PROJECT_SOURCE_DIR}
)

# 链接必要的库
target_link_libraries(client_app pthread hiredis sodium curl glog)

# 设置 C++ sanitize 标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
