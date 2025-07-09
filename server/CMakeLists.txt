cmake_minimum_required(VERSION 3.20)

file(GLOB SERVER_SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_executable(chat_server ${SERVER_SOURCES})

set_target_properties(chat_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

target_link_libraries(chat_server 
    PRIVATE
    Poco::Foundation
    Poco::Net
    Poco::Util
    Poco::JSON
    chat_protocol
)

target_include_directories(chat_server PRIVATE 
    ${POCO_INCLUDE_DIR}
)
