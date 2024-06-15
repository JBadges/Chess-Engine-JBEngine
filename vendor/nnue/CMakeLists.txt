cmake_minimum_required(VERSION 3.10)
project(nnue LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler-specific flags for POPCNT instruction
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-mpopcnt)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/arch:AVX2)
endif()

# Add the nnue library as a shared library
add_library(nnue SHARED
    misc.cpp
    nnue.cpp
)

# Define import/export macros for Windows DLLs
if(WIN32)
    target_compile_definitions(nnue PRIVATE NNUE_EXPORTS)
endif()

# Include directories for nnue
target_include_directories(nnue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
