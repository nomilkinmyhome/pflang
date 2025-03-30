cmake_minimum_required(VERSION 3.10)
project(pflang C)

set(CMAKE_C_STANDARD 11)

# Include directories
include_directories(include)

# Source files (excluding main.c for library)
set(LIB_SOURCES
    src/token.c
    src/lexer.c
    src/ast.c
    src/parser.c
    src/utils.c
    src/test_framework.c
)

# Main executable sources
set(SOURCES
    src/main.c
    ${LIB_SOURCES}
)

# Create main executable
add_executable(pflang ${SOURCES})

# Add compiler warnings
if(MSVC)
    target_compile_options(pflang PRIVATE /W4)
else()
    target_compile_options(pflang PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Create a library for testing
add_library(pflang_lib STATIC ${LIB_SOURCES})

# Test executables
set(TEST_SOURCES
        tests/run_tests.c
        tests/lexer_tests.c
        tests/parser_tests.c
        tests/function_syntax_tests.c
)

add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests pflang_lib)

# Add a custom target to run all tests
add_custom_target(test
    COMMAND run_tests
    DEPENDS run_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
