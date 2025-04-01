#ifndef PFLANG_TOKEN_H
#define PFLANG_TOKEN_H

#include "common.h"

// Token types
typedef enum {
    // Keywords
    TOKEN_FUNCTION,     // 'f'
    TOKEN_RETURN,       // 'return'
    TOKEN_IF,           // 'if'
    TOKEN_ELSIF,        // 'elsif'
    TOKEN_ELSE,         // 'else'
    TOKEN_WHILE,        // 'while'
    TOKEN_FOR,          // 'for'
    TOKEN_BREAK,        // 'break'
    TOKEN_CONTINUE,     // 'continue'
    TOKEN_OPTIONAL,     // 'optional'

    // Types
    TOKEN_U8,
    TOKEN_U16,
    TOKEN_U32,
    TOKEN_U64,
    TOKEN_I8,
    TOKEN_I16,
    TOKEN_I32,
    TOKEN_I64,
    TOKEN_F32,
    TOKEN_F64,
    TOKEN_STR,
    TOKEN_BOOL,
    TOKEN_NULL,
    TOKEN_ERROR,

    // Operators
    TOKEN_PLUS,          // +
    TOKEN_MINUS,         // -
    TOKEN_MULTIPLY,      // *
    TOKEN_DIVIDE,        // /
    TOKEN_MODULO,        // %
    TOKEN_INCREMENT,     // ++
    TOKEN_DECREMENT,     // --
    TOKEN_GREATER,       // >
    TOKEN_GREATER_EQUAL, // >=
    TOKEN_LESS,          // <
    TOKEN_LESS_EQUAL,    // <=
    TOKEN_AND,           // &&
    TOKEN_OR,            // ||

    // Other tokens
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_COLON,         // :
    TOKEN_ARROW,         // ->
    TOKEN_EQUALS,        // ==
    TOKEN_NOT_EQUAL,     // !=
    TOKEN_ASSIGNMENT,    // =
    TOKEN_EOF,
    TOKEN_LEFT_PAREN,    // (
    TOKEN_RIGHT_PAREN,   // )
    TOKEN_LEFT_BRACE,    // {
    TOKEN_RIGHT_BRACE,   // }
    TOKEN_COMMA,         // ,
    TOKEN_DOT,           // .
    TOKEN_TUPLE_TYPE,
} TokenType;

// Token structure
typedef struct Token {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

// Token functions
void free_token(Token* token);
const char* token_type_to_string(TokenType type);

#endif // PFLANG_TOKEN_H
