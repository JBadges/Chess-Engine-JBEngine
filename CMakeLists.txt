cmake_minimum_required(VERSION 3.0.0)
project(JBEngine VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_executable(JBEngine main.cpp bitboard.cpp attacks.cpp position.cpp search.cpp uci.cpp eval.cpp movegenerator.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
