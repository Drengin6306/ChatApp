cmake_minimum_required(VERSION 3.20)

file(GLOB CLIENT_SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_executable(chat_client ${CLIENT_SOURCES})

target_link_libraries(chat_client 
    PRIVATE
    chat_protocol
    Poco::Foundation
    Poco::Net
)

target_include_directories(chat_client 
    PRIVATE 
    src/
    ${CMAKE_SOURCE_DIR}/protocol/src/
)

target_compile_features(chat_client PRIVATE cxx_std_17)
    ${POCO_INCLUDE_DIR}
)
