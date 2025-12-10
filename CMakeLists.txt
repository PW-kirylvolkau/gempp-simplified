cmake_minimum_required(VERSION 3.10)
project(gempp-v2 VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Extract GLPK from archive to build directory if not already extracted
set(GLPK_ARCHIVE ${CMAKE_SOURCE_DIR}/external/glpk-5.0.tar.gz)
set(GLPK_DIR ${CMAKE_BINARY_DIR}/glpk-5.0)

if(NOT EXISTS ${GLPK_DIR})
    message(STATUS "Extracting GLPK from archive...")
    file(ARCHIVE_EXTRACT INPUT ${GLPK_ARCHIVE} DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Generate config.h for GLPK
if(WIN32)
    file(WRITE ${CMAKE_BINARY_DIR}/config.h "/* GLPK config for Windows */\n")
else()
    file(WRITE ${CMAKE_BINARY_DIR}/config.h "#define HAVE_SYS_TIME_H 1\n")
endif()

# Collect all GLPK source files
file(GLOB GLPK_SOURCES
    ${GLPK_DIR}/src/amd/*.c
    ${GLPK_DIR}/src/api/*.c
    ${GLPK_DIR}/src/bflib/*.c
    ${GLPK_DIR}/src/colamd/*.c
    ${GLPK_DIR}/src/draft/*.c
    ${GLPK_DIR}/src/env/*.c
    ${GLPK_DIR}/src/intopt/*.c
    ${GLPK_DIR}/src/minisat/*.c
    ${GLPK_DIR}/src/misc/*.c
    ${GLPK_DIR}/src/mpl/*.c
    ${GLPK_DIR}/src/npp/*.c
    ${GLPK_DIR}/src/proxy/*.c
    ${GLPK_DIR}/src/simplex/*.c
    ${GLPK_DIR}/src/zlib/*.c
)

# Exclude proxy/main.c (it has its own main function)
list(FILTER GLPK_SOURCES EXCLUDE REGEX ".*/proxy/main\\.c$")

# Create GLPK static library
add_library(glpk STATIC ${GLPK_SOURCES})

target_include_directories(glpk PRIVATE
    ${CMAKE_BINARY_DIR}
    ${GLPK_DIR}/src
    ${GLPK_DIR}/src/amd
    ${GLPK_DIR}/src/api
    ${GLPK_DIR}/src/bflib
    ${GLPK_DIR}/src/colamd
    ${GLPK_DIR}/src/draft
    ${GLPK_DIR}/src/env
    ${GLPK_DIR}/src/intopt
    ${GLPK_DIR}/src/minisat
    ${GLPK_DIR}/src/misc
    ${GLPK_DIR}/src/mpl
    ${GLPK_DIR}/src/npp
    ${GLPK_DIR}/src/proxy
    ${GLPK_DIR}/src/simplex
    ${GLPK_DIR}/src/zlib
)

target_compile_definitions(glpk PRIVATE
    HAVE_CONFIG_H=1
    $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

# Suppress warnings for third-party GLPK code
if(MSVC)
    target_compile_options(glpk PRIVATE /W0)
else()
    target_compile_options(glpk PRIVATE -w)
endif()

# Main executable
add_executable(gempp-v2 src/main.cpp)

target_include_directories(gempp-v2 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GLPK_DIR}/src
)

target_link_libraries(gempp-v2 PRIVATE glpk)

if(MSVC)
    target_compile_options(gempp-v2 PRIVATE /W4)
else()
    target_compile_options(gempp-v2 PRIVATE -Wall -Wextra -pedantic)
endif()

install(TARGETS gempp-v2 DESTINATION bin)
