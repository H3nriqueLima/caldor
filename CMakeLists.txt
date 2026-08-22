cmake_minimum_required(VERSION 3.15)
project(caldor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# libcds, header-only, buscada direto do repo (mesmo jeito documentado no README dela).
include(FetchContent)
FetchContent_Declare(
    libcds
    GIT_REPOSITORY https://github.com/H3nriqueLima/libcds.git
    GIT_TAG main
)
FetchContent_MakeAvailable(libcds)

# Diretório de headers públicos do próprio CaldOR.
include_directories(${PROJECT_SOURCE_DIR}/include)

# Fontes do interpretador/compilador.
file(GLOB CALDOR_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(caldor ${CALDOR_SOURCES})

target_include_directories(caldor PRIVATE ${libcds_SOURCE_DIR}/include)

# Warnings mais rigorosos, ajuda a pegar bug cedo.
if (MSVC)
    target_compile_options(caldor PRIVATE /W4)
else()
    target_compile_options(caldor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Testes (opcional, ligado por padrão; desligue com -DCALDOR_BUILD_TESTS=OFF).
option(CALDOR_BUILD_TESTS "Build Caldor test suite" ON)
if (CALDOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()