cmake_minimum_required(VERSION 3.14)
project(gempp VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Required for FetchContent
include(FetchContent)

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

# ============================================
# FTXUI Library (Terminal UI + Canvas)
# ============================================

# Try to use bundled archive first (for offline builds)
set(FTXUI_ARCHIVE ${CMAKE_SOURCE_DIR}/external/ftxui-6.1.9.tar.gz)
if(EXISTS ${FTXUI_ARCHIVE})
    message(STATUS "Using bundled FTXUI archive")
    FetchContent_Declare(ftxui
        URL ${FTXUI_ARCHIVE}
    )
else()
    message(STATUS "Fetching FTXUI from GitHub")
    FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v6.1.9
    )
endif()

# Disable FTXUI extras we don't need
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui)

# Main executable - output to project root
add_executable(gempp src/main.cpp)

# Set output directory to project root
set_target_properties(gempp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}
)

target_include_directories(gempp PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GLPK_DIR}/src
)

target_link_libraries(gempp PRIVATE
    glpk
    ftxui::screen
    ftxui::dom
    ftxui::component
)

if(MSVC)
    target_compile_options(gempp PRIVATE /W4)
else()
    target_compile_options(gempp PRIVATE -Wall -Wextra -pedantic)
endif()

install(TARGETS gempp DESTINATION bin)
