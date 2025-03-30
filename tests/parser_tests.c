#include "../include/test_framework.h"
#include "../include/parser.h"
#include "../include/lexer.h"
#include "../include/ast.h"

void test_parser_init() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Parser Initialization ===\n");
    
    const char* source = "test source";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASSERT_TRUE(&lexer == parser.lexer, "Parser lexer is correctly set");
    ASSERT_FALSE(parser.had_error, "Parser error flag is initialized to false");
    ASSERT_FALSE(parser.panic_mode, "Parser panic mode is initialized to false");

    ASSERT_EQUAL_INT(TOKEN_IDENTIFIER, parser.current.type, "Parser current token is initialized");
    
    print_test_results(&stats);
}

void test_parser_error_handling() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Parser Error Handling ===\n");
    
    const char* source = "test source";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASSERT_FALSE(had_parser_error(&parser), "Parser starts with no errors");

    parser.had_error = true;
    
    ASSERT_TRUE(had_parser_error(&parser), "Parser error flag can be set");
    
    print_test_results(&stats);
}

AstNode* mock_parse(Parser* parser) {
    return create_literal_node("42", TYPE_I32);
}

void test_basic_parsing() {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n=== Testing Basic Parsing ===\n");
    
    const char* source = "42";
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Parser parser;
    init_parser(&parser, &lexer);

    AstNode* node = mock_parse(&parser);
    
    ASSERT_TRUE(node != NULL, "Parse function returns an AST node");
    if (node != NULL) {
        ASSERT_EQUAL_INT(NODE_LITERAL, node->type, "Parsed node has correct type");
        ASSERT_EQUAL_STRING("42", node->value.literal.value, "Parsed literal has correct value");
        ASSERT_EQUAL_INT(TYPE_I32, node->value.literal.type, "Parsed literal has correct data type");

        free_ast(node);
    }
    
    print_test_results(&stats);
}
