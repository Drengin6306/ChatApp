cmake_minimum_required(VERSION 3.20)
project(ChatApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

find_package(Poco REQUIRED COMPONENTS Foundation Net Util JSON)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(server)
add_subdirectory(client)

add_custom_target(copy_config ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        ${CMAKE_SOURCE_DIR}/bin/config
    COMMENT "Copying config files to bin directory"
)
