cmake_minimum_required(VERSION 3.15)

# 收集源文件
file(GLOB SERVER_SOURCES
    "src/*.cpp"
    "src/*.h"
)

# 创建服务器可执行文件
add_executable(chat_server ${SERVER_SOURCES})

# 设置可执行文件输出目录到顶层bin目录
set_target_properties(chat_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

# 链接库
target_link_libraries(chat_server 
    ${POCO_FOUNDATION}
    ${POCO_NET}
    ${POCO_UTIL}
    ${POCO_JSON}
    pthread
)

# 设置包含目录
target_include_directories(chat_server PRIVATE 
    ${POCO_INCLUDE_DIR}
    inc/
)
