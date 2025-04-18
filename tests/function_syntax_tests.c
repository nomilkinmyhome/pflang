#include "../include/test_framework.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/ast.h"

// Function prototypes
void test_simple_function_with_null();
void test_function_with_multiple_returns();
void test_function_with_error_handling();
void test_function_with_different_return_types();
void test_function_with_simple_number_return();
void test_function_with_string_return();
void test_function_with_variable_declarations();
void test_function_with_invalid_variable_declaration();

void run_function_syntax_test(const char* source, const char* test_name, TestStats* stats) {
    printf("\n--- Testing: %s ---\n", test_name);
    printf("Source code:\n%s\n", source);

    Lexer lexer;
    init_lexer(&lexer, source);
    
    Parser parser;
    init_parser(&parser, &lexer);

    AstNode* ast = parse(&parser);

    if (ast != NULL) {
        stats->passed++;
        printf("AST Structure:\n");
        print_ast(ast, 0);
        printf("\n");

        free_ast(ast);
    } else {
        stats->failed++;
        printf("Failed to create AST\n");
    }

    if (had_parser_error(&parser)) {
        // For tests that expect errors, we don't count this as a failure
        if (strstr(test_name, "Expected Error") != NULL) {
            stats->passed++;
            printf("Parser reported expected errors\n");
        } else {
            stats->failed++;
            printf("Parser reported errors\n");
        }
    } else {
        if (strstr(test_name, "Expected Error") != NULL) {
            stats->failed++;
            printf("Parser did not report expected errors\n");
        } else {
            stats->passed++;
        }
    }

    stats->total = stats->passed + stats->failed;
}

// Test simple function with null return
void test_simple_function_with_null() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = "f main() -> null:\n    return null";
    run_function_syntax_test(test, "Simple function with null return", &stats);
    
    print_test_results(&stats);
}

// Test function with multiple return values
void test_function_with_multiple_returns() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = "f div(a: int, b: int) -> (int, error):\n    return (a / b, null)";
    run_function_syntax_test(test, "Function with multiple return values", &stats);
    
    print_test_results(&stats);
}

// Test function with error handling
void test_function_with_error_handling() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = 
        "f div(a: int, b: int) -> (int, error):\n"
        "    if b == 0:\n"
        "        return (0, error(\"Division by zero\"))\n"
        "    return (a / b, null)";
    run_function_syntax_test(test, "Function with error handling", &stats);
    
    print_test_results(&stats);
}

// Test function with different return types
void test_function_with_different_return_types() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = "f process() -> (i32, str, error):\n    return (42, \"success\", null)";
    run_function_syntax_test(test, "Function with different return types", &stats);
    
    print_test_results(&stats);
}

// Test function with simple number return
void test_function_with_simple_number_return() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = "f get_age() -> u8:\n    return 25";
    run_function_syntax_test(test, "Function with simple number return", &stats);
    
    print_test_results(&stats);
}

// Test function with string return
void test_function_with_string_return() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = "f get_name() -> str:\n    return \"John\"";
    run_function_syntax_test(test, "Function with string return", &stats);
    
    print_test_results(&stats);
}

// Test function with variable declarations
void test_function_with_variable_declarations() {
    TestStats stats;
    init_test_stats(&stats);
    
    const char* test = 
        "f some_func(a: u8, b: str) -> null:\n"
        "    int first_operand = 1\n"
        // "    u8 second_operand = u8(2)\n"  TODO: add later
        "    optional int maybe_value = null\n"
        "    return null";
    run_function_syntax_test(test, "Function with variable declarations", &stats);
    
    print_test_results(&stats);
}
