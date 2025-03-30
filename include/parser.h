#ifndef PFLANG_PARSER_H
#define PFLANG_PARSER_H

#include "common.h"
#include "token.h"
#include "lexer.h"
#include "ast.h"

// Symbol table entry
typedef struct Symbol {
    char* name;
    DataType type;
    // Additional info like line number, value, etc.
} Symbol;

// Parser structure
typedef struct Parser {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

// Parser functions
void init_parser(Parser* parser, Lexer* lexer);
AstNode* parse(Parser* parser);
bool had_parser_error(Parser* parser);
static AstNode* parse_expression(Parser* parser);
static AstNode* parse_statement(Parser* parser);
static AstNode* parse_declaration(Parser* parser);
static AstNode* parse_return_statement(Parser* parser);
static AstNode* parse_parameter(Parser* parser);
static AstNode* parse_primary(Parser* parser);
static AstNode* parse_unary(Parser* parser);
static AstNode* parse_factor(Parser* parser);
static AstNode* parse_term(Parser* parser);
static AstNode* parse_comparison(Parser* parser);
static AstNode* parse_equality(Parser* parser);
static AstNode* parse_if_statement(Parser* parser);

#endif // PFLANG_PARSER_H
