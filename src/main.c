#include <stdio.h>
#include <stdlib.h>
#include "../include/common.h"
#include "../include/token.h"
#include "../include/lexer.h"
#include "../include/ast.h"
#include "../include/parser.h"
#include "../include/utils.h"

// Implementation of data_type_to_string
const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_STR: return "str";
        case TYPE_BOOL: return "bool";
        case TYPE_NULL: return "null";
        case TYPE_ERROR: return "error";
        case TYPE_TUPLE: return "tuple";
        default: return "unknown";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: pflang [file]\n");
        exit(64);
    }

    char* source = read_file(argv[1]);
    
    Lexer lexer;
    init_lexer(&lexer, source);
    
    Parser parser;
    init_parser(&parser, &lexer);

    printf("Tokens:\n");
    Token token;
    do {
        token = scan_token(&lexer);
        printf("%s: '%s' at line %d, column %d\n", 
               token_type_to_string(token.type), 
               token.lexeme, 
               token.line, 
               token.column);
        free_token(&token);
    } while (token.type != TOKEN_EOF);
    
    free(source);
    return 0;
}
