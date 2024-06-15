cmake_minimum_required(VERSION 3.10)
project(ChessEngine VERSION 1.0.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler-specific flags for POPCNT instruction
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-mpopcnt)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/arch:AVX2)
endif()

# Add the nnue library
add_subdirectory(vendor/nnue)

# Add the Fathom library
add_subdirectory(vendor/Fathom)

# Define paths for NNUE file and Syzygy tablebases
set(NNUE_FILE_PATH "${CMAKE_SOURCE_DIR}/nn-eba324f53044.nnue")
set(SYZYGY_PATH "${CMAKE_SOURCE_DIR}/syzygy/345")

# Add the executable
add_executable(ChessEngine
    src/attacks.cpp
    src/bitboard.cpp
    src/eval.cpp
    src/main.cpp
    src/movegenerator.cpp
    src/position.cpp
    src/search.cpp
    src/transpositiontable.cpp
    src/uci.cpp
)

# Define import/export macros for Windows DLLs
if(WIN32)
    target_compile_definitions(ChessEngine PRIVATE NNUE_IMPORTS)
endif()

# Link the libraries
target_link_libraries(ChessEngine nnue fathom)

# Include directories for ChessEngine
target_include_directories(ChessEngine PRIVATE
    ${PROJECT_SOURCE_DIR}/vendor/nnue
    ${PROJECT_SOURCE_DIR}/vendor/Fathom/src
)

# Pass the paths to the source code
target_compile_definitions(ChessEngine PRIVATE
    NNUE_FILE_PATH="${NNUE_FILE_PATH}"
    SYZYGY_PATH="${SYZYGY_PATH}"
)

# Ensure DLLs are copied to the output directory
if(WIN32)
    add_custom_command(TARGET ChessEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:nnue>
            $<TARGET_FILE_DIR:ChessEngine>
    )
endif()
