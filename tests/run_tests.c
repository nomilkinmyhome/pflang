#include <stdio.h>

// Lexer test functions
extern void test_lexer_init();
extern void test_basic_tokens();
extern void test_keywords();
extern void test_literals();
extern void test_whitespace_comments();
extern void test_line_column_tracking();

// Parser test functions
extern void test_parser_init();
extern void test_parser_error_handling();
extern void test_basic_parsing();

// Function syntax test functions
extern void test_simple_function_with_null();
extern void test_function_with_multiple_returns();
extern void test_function_with_error_handling();
extern void test_function_with_different_return_types();
extern void test_function_with_simple_number_return();
extern void test_function_with_string_return();

int main() {
    printf("==============================\n");
    printf("Running all pflang tests\n");
    printf("==============================\n\n");

    // Run function syntax tests
    printf("\n==============================\n");
    printf("FUNCTION SYNTAX TESTS\n");
    printf("==============================\n");
    test_simple_function_with_null();
    test_function_with_multiple_returns();
    test_function_with_error_handling();
    test_function_with_different_return_types();
    test_function_with_simple_number_return();
    test_function_with_string_return();

    /*
    // Run lexer tests
    printf("\n==============================\n");
    printf("LEXER TESTS\n");
    printf("==============================\n");
    test_lexer_init();
    test_basic_tokens();
    test_keywords();
    test_literals();
    test_whitespace_comments();
    test_line_column_tracking();
    
    // Run parser tests
    printf("\n==============================\n");
    printf("PARSER TESTS\n");
    printf("==============================\n");
    test_parser_init();
    test_parser_error_handling();
    test_basic_parsing();
    */
    printf("\n==============================\n");
    printf("All tests completed\n");
    printf("==============================\n");
    
    return 0;
}
