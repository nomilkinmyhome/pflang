#ifndef PFLANG_AST_H
#define PFLANG_AST_H

#include "common.h"
#include "token.h"

// AST node types
typedef enum {
    NODE_FUNCTION,
    NODE_BLOCK,
    NODE_RETURN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_PARAMETER,
    NODE_VARIABLE,
    NODE_LITERAL,
    NODE_TUPLE,
    NODE_FUNCTION_CALL,
} NodeType;

// AST node structure
typedef struct AstNode {
    NodeType type;
    DataType data_type;
    union {
        // Function declaration
        struct {
            char* name;
            struct AstNode** parameters;
            int param_count;
            struct AstNode* body;
            DataType* return_types;
            int return_type_count;
        } function;

        // Variable declaration/reference
        struct {
            char* name;
            struct AstNode* init_value;
            DataType type;
            bool is_optional;
        } variable;

        // Binary operation
        struct {
            struct AstNode* left;
            struct AstNode* right;
            TokenType operator;
        } binary_op;

        // Return statement
        struct {
            struct AstNode* return_value;
        } return_stmt;

        // Tuple values
        struct {
            struct AstNode** values;
            int value_count;
        } tuple;

        // Literal value
        struct {
            char* value;
            DataType type;
        } literal;

        // Function parameter
        struct {
            char* name;
            DataType type;
        } parameter;

        // Function call
        struct {
            char* name;
            struct AstNode** arguments;
            int argument_count;
        } function_call;

        // Unary operator
        struct {
            TokenType operator;
            struct AstNode* operand;
        } unary_op;

        // If statement
        struct {
            struct AstNode* condition;
            struct AstNode** then_branches;
            int then_branches_count;
            struct AstNode* else_branch;
        } if_stmt;

        // Block of statements
        struct {
            struct AstNode** statements;
            int statement_count;
        } block;
    } value;
} AstNode;

// AST functions
AstNode* create_function_node(char* name, AstNode** parameters, int param_count, 
                             AstNode* body, DataType* return_types, int return_type_count);
AstNode* create_variable_node(char* name, AstNode* init_value, DataType type, bool is_optional);
AstNode* create_binary_op_node(AstNode* left, AstNode* right, TokenType operator);
AstNode* create_return_node(AstNode* return_value);
AstNode* create_tuple_node(AstNode** values, int value_count);
AstNode* create_literal_node(char* value, DataType type);
AstNode* create_parameter_node(char* name, DataType type);
AstNode* create_unary_op_node(TokenType operator, AstNode* operand);
void free_ast(AstNode* node);

// Print AST node and its children with indentation
void print_ast(AstNode* node, int indent_level);

#endif // PFLANG_AST_H
