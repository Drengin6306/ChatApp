cmake_minimum_required(VERSION 3.20)

file(GLOB CLIENT_SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_executable(chat_client ${CLIENT_SOURCES})

set_target_properties(chat_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

target_link_libraries(chat_client 
    PRIVATE
    Poco::Foundation
    Poco::Net
    Poco::Util
)

target_include_directories(chat_client PRIVATE 
    ${POCO_INCLUDE_DIR}
    inc/
)
