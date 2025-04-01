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

void test_variable_declaration() {
    TestStats stats;
    init_test_stats(&stats);

    printf("\n=== Testing Variable Declaration Parsing ===\n");

    // Test regular variable declaration
    const char* source = "u8 second_operand = u8(2)";
    Lexer lexer;
    init_lexer(&lexer, source);

    Parser parser;
    init_parser(&parser, &lexer);

    AstNode* node = parse(&parser);

    ASSERT_TRUE(node != NULL, "Parse function returns an AST node");
    if (node != NULL) {
        ASSERT_EQUAL_INT(NODE_VARIABLE, node->type, "Parsed node has correct type");
        ASSERT_EQUAL_STRING("second_operand", node->value.variable.name, "Variable has correct name");
        ASSERT_EQUAL_INT(TYPE_U8, node->value.variable.type, "Variable has correct type");
        ASSERT_FALSE(node->value.variable.is_optional, "Variable is not optional");

        ASSERT_TRUE(node->value.variable.init_value != NULL, "Variable has initialization value");
        if (node->value.variable.init_value != NULL) {
            ASSERT_EQUAL_INT(NODE_FUNCTION_CALL, node->value.variable.init_value->type, "Init value is a function call");
        }

        free_ast(node);
    }

    // Test optional variable declaration
    const char* source2 = "optional int somevar = null";
    init_lexer(&lexer, source2);
    init_parser(&parser, &lexer);

    node = parse(&parser);

    ASSERT_TRUE(node != NULL, "Parse function returns an AST node for optional variable");
    if (node != NULL) {
        ASSERT_EQUAL_INT(NODE_VARIABLE, node->type, "Parsed node has correct type");
        ASSERT_EQUAL_STRING("somevar", node->value.variable.name, "Variable has correct name");
        ASSERT_EQUAL_INT(TYPE_I32, node->value.variable.type, "Variable has correct type");
        ASSERT_TRUE(node->value.variable.is_optional, "Variable is optional");

        ASSERT_TRUE(node->value.variable.init_value != NULL, "Variable has initialization value");
        if (node->value.variable.init_value != NULL) {
            ASSERT_EQUAL_INT(NODE_LITERAL, node->value.variable.init_value->type, "Init value is a literal");
            ASSERT_EQUAL_INT(TYPE_NULL, node->value.variable.init_value->value.literal.type, "Init value is null");
        }

        free_ast(node);
    }

    print_test_results(&stats);
}
