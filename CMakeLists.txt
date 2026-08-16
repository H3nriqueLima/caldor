cmake_minimum_required(VERSION 3.15)
project(caldor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Diretório de headers públicos
include_directories(${PROJECT_SOURCE_DIR}/include)

# Fontes do interpretador/compilador
file(GLOB CALDOR_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(caldor ${CALDOR_SOURCES})

if (MSVC)
    target_compile_options(caldor PRIVATE /W4)
else()
    target_compile_options(caldor PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(CALDOR_BUILD_TESTS "Build Caldor test suite" ON)
if (CALDOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()