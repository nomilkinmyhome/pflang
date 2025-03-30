#ifndef PFLANG_LEXER_H
#define PFLANG_LEXER_H

#include "common.h"
#include "token.h"

// Lexer structure
typedef struct Lexer {
    const char* source;
    int start;
    int current;
    int line;
    int column;
} Lexer;

// Lexer functions
void init_lexer(Lexer* lexer, const char* source);
Token scan_token(Lexer* lexer);

#endif // PFLANG_LEXER_H
