#ifndef PFLANG_COMMON_H
#define PFLANG_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Forward declarations
typedef struct Token Token;
typedef struct Lexer Lexer;
typedef struct AstNode AstNode;
typedef struct Parser Parser;
typedef struct Symbol Symbol;

// Type system
typedef enum {
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_F32,
    TYPE_F64,
    TYPE_STR,
    TYPE_BOOL,
    TYPE_NULL,
    TYPE_ERROR,
    TYPE_TUPLE,
    TYPE_ARRAY,
    TYPE_LIST,
    TYPE_MAP,
} DataType;

#endif // PFLANG_COMMON_H
