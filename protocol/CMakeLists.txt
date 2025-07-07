cmake_minimum_required(VERSION 3.20)

file(GLOB PROTOCOL_SOURCES
    "src/*.cpp"
    "inc/*.h"
)

add_library(chat_protocol STATIC ${PROTOCOL_SOURCES})

set_target_properties(chat_protocol PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(chat_protocol 
    PUBLIC
    Poco::Foundation
    Poco::JSON
)

target_include_directories(chat_protocol 
    PUBLIC 
    src/
    PRIVATE
    ${POCO_INCLUDE_DIR}
)

target_compile_features(chat_protocol PUBLIC cxx_std_17)