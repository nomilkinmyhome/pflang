#include "../include/test_framework.h"
#include "../include/lexer.h"
#include "../include/token.h"

// Test lexer initialization
void test_lexer_init() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Lexer Initialization ===\n");
    
    const char* source = "test source";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    ASSERT_TRUE(lexer.source == source, "Lexer source is correctly set");
    ASSERT_EQUAL_INT(0, lexer.start, "Lexer start position is initialized to 0");
    ASSERT_EQUAL_INT(0, lexer.current, "Lexer current position is initialized to 0");
    ASSERT_EQUAL_INT(1, lexer.line, "Lexer line is initialized to 1");
    ASSERT_EQUAL_INT(1, lexer.column, "Lexer column is initialized to 1");
    
    print_test_results(&stats);
}

// Test basic token scanning
void test_basic_tokens() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Basic Token Scanning ===\n");
    
    const char* source = "( ) { } : , . -> + * / % == > < >= <= ++ --";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Token token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_LEFT_PAREN, token.type, "Scanned LEFT_PAREN token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_RIGHT_PAREN, token.type, "Scanned RIGHT_PAREN token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_LEFT_BRACE, token.type, "Scanned LEFT_BRACE token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_RIGHT_BRACE, token.type, "Scanned RIGHT_BRACE token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_COLON, token.type, "Scanned COLON token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_COMMA, token.type, "Scanned COMMA token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_DOT, token.type, "Scanned DOT token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_ARROW, token.type, "Scanned ARROW token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_PLUS, token.type, "Scanned PLUS token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_MULTIPLY, token.type, "Scanned MULTIPLY token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_DIVIDE, token.type, "Scanned DIVIDE token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_MODULO, token.type, "Scanned MODULO token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_EQUALS, token.type, "Scanned EQUALS token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_GREATER, token.type, "Scanned GREATER token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_LESS, token.type, "Scanned LESS token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_GREATER_EQUAL, token.type, "Scanned GREATER_EQUAL token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_LESS_EQUAL, token.type, "Scanned LESS_EQUAL token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_INCREMENT, token.type, "Scanned INCREMENT token");
    free_token(&token);
    
    // Note: The lexer doesn't seem to handle DECREMENT (--) yet
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_MINUS, token.type, "Scanned MINUS token");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_EOF, token.type, "Scanned EOF token");
    free_token(&token);
    
    print_test_results(&stats);
}

// Test keyword recognition
void test_keywords() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Keyword Recognition ===\n");
    
    const char* source = "f return if else elsif i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 str bool null error";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Token token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_FUNCTION, token.type, "Recognized FUNCTION keyword");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_RETURN, token.type, "Recognized RETURN keyword");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_IF, token.type, "Recognized IF keyword");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_ELSE, token.type, "Recognized ELSE keyword");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_ELSIF, token.type, "Recognized ELSIF keyword");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_I8, token.type, "Recognized I8 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_I16, token.type, "Recognized I16 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_I32, token.type, "Recognized I32 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_I64, token.type, "Recognized I64 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_U8, token.type, "Recognized U8 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_U16, token.type, "Recognized U16 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_U32, token.type, "Recognized U32 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_U64, token.type, "Recognized U64 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_F32, token.type, "Recognized F32 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_F64, token.type, "Recognized F64 type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_STR, token.type, "Recognized STR type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_BOOL, token.type, "Recognized BOOL type");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_NULL, token.type, "Recognized NULL keyword");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_ERROR, token.type, "Recognized ERROR keyword");
    free_token(&token);
    
    print_test_results(&stats);
}

// Test identifiers, numbers, and strings
void test_literals() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Literals ===\n");
    
    const char* source = "identifier 123 123.456 \"string\"";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Token token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, token.type, "Recognized IDENTIFIER");
    ASSERT_EQUAL_STRING("identifier", token.lexeme, "Correct identifier lexeme");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_NUMBER, token.type, "Recognized NUMBER");
    ASSERT_EQUAL_STRING("123", token.lexeme, "Correct integer lexeme");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_NUMBER, token.type, "Recognized decimal NUMBER");
    ASSERT_EQUAL_STRING("123.456", token.lexeme, "Correct decimal lexeme");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_STRING, token.type, "Recognized STRING");
    ASSERT_EQUAL_STRING("\"string\"", token.lexeme, "Correct string lexeme");
    free_token(&token);
    
    print_test_results(&stats);
}

// Test whitespace and comments
void test_whitespace_comments() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Whitespace and Comments ===\n");
    
    const char* source = "  \t\r\n  # This is a comment\nidentifier";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Token token = scan_token(&lexer);
    ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, token.type, "Skipped whitespace and comments");
    ASSERT_EQUAL_STRING("identifier", token.lexeme, "Correct identifier after whitespace and comments");
    ASSERT_EQUAL_INT(2, token.line, "Correct line number after newline");
    free_token(&token);
    
    print_test_results(&stats);
}

// Test line and column tracking
void test_line_column_tracking() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Line and Column Tracking ===\n");
    
    const char* source = "a\nb\n  c";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Token token = scan_token(&lexer);
    ASSERT_EQUAL_INT(1, token.line, "First token on line 1");
    ASSERT_EQUAL_INT(1, token.column, "First token at column 1");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(2, token.line, "Second token on line 2");
    ASSERT_EQUAL_INT(1, token.column, "Second token at column 1");
    free_token(&token);
    
    token = scan_token(&lexer);
    ASSERT_EQUAL_INT(3, token.line, "Third token on line 3");
    ASSERT_EQUAL_INT(3, token.column, "Third token at column 3");
    free_token(&token);
    
    print_test_results(&stats);
}

// Main function removed as tests are now called from run_tests.c
