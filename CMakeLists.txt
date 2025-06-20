cmake_minimum_required(VERSION 3.15)
project(ChatApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Poco 库
find_library(POCO_FOUNDATION PocoFoundation REQUIRED)
find_library(POCO_NET PocoNet REQUIRED)
find_library(POCO_UTIL PocoUtil REQUIRED)
find_library(POCO_JSON PocoJSON REQUIRED)

include_directories(${POCO_INCLUDE_DIR})

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(server)
add_subdirectory(client)

add_custom_target(copy_config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/bin/config
    COMMENT "Copying config files to bin directory"
)
