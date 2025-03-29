﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Token types
typedef enum {
    // Keywords
    TOKEN_FUNCTION,    // 'f'
    TOKEN_RETURN,      // 'return'
    TOKEN_IF,          // 'if'
    TOKEN_ELSIF,       // 'elsif'
    TOKEN_ELSE,        // 'else'
    TOKEN_WHILE,       // 'while'
    TOKEN_FOR,         // 'for'
    TOKEN_BREAK,       // 'break'
    TOKEN_CONTINUE,    // 'continue'

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
    TOKEN_PLUS,        // +
    TOKEN_MINUS,       // -
    TOKEN_MULTIPLY,    // *
    TOKEN_DIVIDE,      // /
    TOKEN_MODULO,      // %
    TOKEN_INCREMENT,   // ++
    TOKEN_DECREMENT,   // --

    // Other tokens
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_COLON,       // :
    TOKEN_ARROW,       // ->
    TOKEN_EQUALS,      // =
    TOKEN_EOF,
    TOKEN_LEFT_PAREN,    // (
    TOKEN_RIGHT_PAREN,   // )
    TOKEN_LEFT_BRACE,    // {
    TOKEN_RIGHT_BRACE,   // }
    TOKEN_COMMA,         // ,
    TOKEN_DOT,          // .
    TOKEN_TUPLE_TYPE,
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

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
    NODE_VARIABLE,
    NODE_LITERAL,
    NODE_TUPLE,
} NodeType;

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
    TYPE_ARRAY,
    TYPE_LIST,
    TYPE_MAP,
    TYPE_TUPLE,
    TYPE_ERROR,
} DataType;

// Symbol table entry
typedef struct {
    char* name;
    DataType type;
    int scope_level;
    // Additional info like line number, value, etc.
} Symbol;

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
    } value;
} AstNode;

typedef struct {
    const char* source;
    int start;
    int current;
    int line;
    int column;
} Lexer;

void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
}

static bool is_at_end(Lexer* lexer) {
    return lexer->source[lexer->current] == '\0';
}

static char advance_lexer(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->source[lexer->current - 1];
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current + 1];
}

static bool match_lexer(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;

    lexer->current++;
    lexer->column++;
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.line = lexer->line;
    token.column = lexer->column - (lexer->current - lexer->start);

    int length = lexer->current - lexer->start;
    token.lexeme = (char*)malloc(length + 1);
    strncpy(token.lexeme, &lexer->source[lexer->start], length);
    token.lexeme[length] = '\0';

    return token;
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance_lexer(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 1;
                advance_lexer(lexer);
                break;
            case '#':  // Comments
                while (peek(lexer) != '\n' && !is_at_end(lexer)) advance_lexer(lexer);
                break;
            default:
                return;
        }
    }
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        advance_lexer(lexer);
    }

    if (is_at_end(lexer)) {
        // Error handling for unterminated string
        Token token = make_token(lexer, TOKEN_EOF);
        token.lexeme = NULL;
        return token;
    }

    // The closing quote
    advance_lexer(lexer);
    return make_token(lexer, TOKEN_STRING);
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static Token number(Lexer* lexer) {
    while (is_digit(peek(lexer))) advance_lexer(lexer);

    // Look for a decimal point
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        advance_lexer(lexer); // Consume the "."
        while (is_digit(peek(lexer))) advance_lexer(lexer);
    }

    return make_token(lexer, TOKEN_NUMBER);
}

static TokenType check_keyword(Lexer* lexer, int start, int length,
                               const char* rest, TokenType type) {
    if (lexer->current - lexer->start == start + length &&
        memcmp(&lexer->source[lexer->start + start], rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->source[lexer->start]) {
        case 'f': return check_keyword(lexer, 1, 0, "", TOKEN_FUNCTION);
        case 'r': return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 'i': return check_keyword(lexer, 1, 1, "f", TOKEN_IF);
        case 'n': return check_keyword(lexer, 1, 3, "ull", TOKEN_NULL);
        case 'e':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'l':
                        if (lexer->current - lexer->start > 2) {
                            if (lexer->source[lexer->start + 2] == 's') {
                                return check_keyword(lexer, 2, 3, "sif", TOKEN_ELSIF);
                            } else {
                                return check_keyword(lexer, 2, 2, "se", TOKEN_ELSE);
                            }
                        }
                        break;
                    case 'r':
                        return check_keyword(lexer, 1, 4, "rror", TOKEN_ERROR);
                }
            }
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier(Lexer* lexer) {
    while (is_alpha(peek(lexer)) || is_digit(peek(lexer))) advance_lexer(lexer);
    return make_token(lexer, identifier_type(lexer));
}

Token scan_token(Lexer* lexer) {
    skip_whitespace(lexer);
    lexer->start = lexer->current;

    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);

    char c = advance_lexer(lexer);

    if (is_alpha(c)) return identifier(lexer);
    if (is_digit(c)) return number(lexer);

    switch (c) {
        case '(': return make_token(lexer, TOKEN_LEFT_PAREN);
        case ')': return make_token(lexer, TOKEN_RIGHT_PAREN);
        case '{': return make_token(lexer, TOKEN_LEFT_BRACE);
        case '}': return make_token(lexer, TOKEN_RIGHT_BRACE);
        case ':': return make_token(lexer, TOKEN_COLON);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '.': return make_token(lexer, TOKEN_DOT);
        case '-':
            if (match_lexer(lexer, '>')) return make_token(lexer, TOKEN_ARROW);
            return make_token(lexer, TOKEN_MINUS);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '*': return make_token(lexer, TOKEN_MULTIPLY);
        case '/': return make_token(lexer, TOKEN_DIVIDE);
        case '"': return string(lexer);
    }

    return make_token(lexer, TOKEN_EOF);
}

void free_token(Token* token) {
    if (token->lexeme != NULL) {
        free(token->lexeme);
        token->lexeme = NULL;
    }
}

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

static void advance_parser(Parser* parser) {
    parser->previous = parser->current;
    parser->current = scan_token(parser->lexer);
}

static void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    // get first token
    advance_parser(parser);
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match_parser(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance_parser(parser);
    return true;
}

static void error_at_current(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;

    fprintf(stderr, "[line %d] Error at '%s': %s\n",
            parser->current.line,
            parser->current.lexeme,
            message);
}

static void error(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;

    fprintf(stderr, "[line %d] Error at '%s': %s\n",
            parser->previous.line,
            parser->previous.lexeme,
            message);
}

static AstNode* parse_expression(Parser* parser);
static AstNode* parse_statement(Parser* parser);
static AstNode* parse_declaration(Parser* parser);
static AstNode* parse_return_statement(Parser* parser);

static AstNode* parse_function(Parser* parser) {
    if (!match_parser(parser, TOKEN_FUNCTION)) {
        error(parser, "Expected 'f' keyword");
        return NULL;
    }

    AstNode* node = malloc(sizeof(AstNode));
    node->type = NODE_FUNCTION;

    if (!match_parser(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected function name");
        return NULL;
    }

    node->value.function.name = strdup(parser->previous.lexeme);

    if (!match_parser(parser, TOKEN_LEFT_PAREN)) {
        error(parser, "Expected '(' after function name");
        return NULL;
    }

    node->value.function.parameters = NULL;
    node->value.function.param_count = 0;

    if (!check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            // TODO: Парсинг параметров
        } while (match_parser(parser, TOKEN_COMMA));
    }

    if (!match_parser(parser, TOKEN_RIGHT_PAREN)) {
        error(parser, "Expected ')' after parameters");
        return NULL;
    }

    if (!match_parser(parser, TOKEN_ARROW)) {
        error(parser, "Expected '->' after parameters");
        return NULL;
    }

    if (match_parser(parser, TOKEN_LEFT_PAREN)) {
        node->value.function.return_types = malloc(sizeof(DataType) * 8);
        node->value.function.return_type_count = 0;
        int capacity = 8;

        do {
            if (node->value.function.return_type_count == capacity) {
                capacity *= 2;
                node->value.function.return_types = realloc(
                        node->value.function.return_types,
                        sizeof(DataType) * capacity
                );
            }

            if (match_parser(parser, TOKEN_NULL)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_NULL;
            } else if (match_parser(parser, TOKEN_ERROR)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_ERROR;
            } else if (match_parser(parser, TOKEN_BOOL)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_BOOL;
            } else if (match_parser(parser, TOKEN_I8)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_I8;
            } else if (match_parser(parser, TOKEN_I16)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_I16;
            } else if (match_parser(parser, TOKEN_I32)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_I32;
            } else if (match_parser(parser, TOKEN_I64)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_I64;
            } else if (match_parser(parser, TOKEN_F32)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_F32;
            } else if (match_parser(parser, TOKEN_F64)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_F64;
            } else if (match_parser(parser, TOKEN_STR)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_STR;
            } else if (match_parser(parser, TOKEN_U8)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_U8;
            } else if (match_parser(parser, TOKEN_U16)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_U16;
            } else if (match_parser(parser, TOKEN_U32)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_U32;
            } else if (match_parser(parser, TOKEN_U64)) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_U64;
            } else {
                error(parser, "Expected return type");
                return NULL;
            }

        } while (match_parser(parser, TOKEN_COMMA));

        if (!match_parser(parser, TOKEN_RIGHT_PAREN)) {
            error(parser, "Expected ')' after return types");
            return NULL;
        }
    } else {
        node->value.function.return_types = malloc(sizeof(DataType));
        node->value.function.return_type_count = 1;

        if (match_parser(parser, TOKEN_NULL)) {
            node->value.function.return_types[0] = TYPE_NULL;
        } else if (match_parser(parser, TOKEN_ERROR)) {
            node->value.function.return_types[0] = TYPE_ERROR;
        } else if (match_parser(parser, TOKEN_BOOL)) {
            node->value.function.return_types[0] = TYPE_BOOL;
        } else if (match_parser(parser, TOKEN_I8)) {
            node->value.function.return_types[0] = TYPE_I8;
        } else if (match_parser(parser, TOKEN_I16)) {
            node->value.function.return_types[0] = TYPE_I16;
        } else if (match_parser(parser, TOKEN_I32)) {
            node->value.function.return_types[0] = TYPE_I32;
        } else if (match_parser(parser, TOKEN_I64)) {
            node->value.function.return_types[0] = TYPE_I64;
        } else if (match_parser(parser, TOKEN_F32)) {
            node->value.function.return_types[0] = TYPE_F32;
        } else if (match_parser(parser, TOKEN_F64)) {
            node->value.function.return_types[0] = TYPE_F64;
        } else if (match_parser(parser, TOKEN_STR)) {
            node->value.function.return_types[0] = TYPE_STR;
        } else if (match_parser(parser, TOKEN_U8)) {
            node->value.function.return_types[0] = TYPE_U8;
        } else if (match_parser(parser, TOKEN_U16)) {
            node->value.function.return_types[0] = TYPE_U16;
        } else if (match_parser(parser, TOKEN_U32)) {
            node->value.function.return_types[0] = TYPE_U32;
        } else if (match_parser(parser, TOKEN_U64)) {
            node->value.function.return_types[0] = TYPE_U64;
        } else {
            error(parser, "Expected return type");
            return NULL;
        }
    }

    if (!match_parser(parser, TOKEN_COLON)) {
        error(parser, "Expected ':' after return type");
        return NULL;
    }

    node->value.function.body = parse_statement(parser);

    return node;
}

static AstNode* parse_expression(Parser* parser) {
    // TODO: Implement expression parsing
    return NULL;
}

static AstNode* parse_statement(Parser* parser) {
    if (match_parser(parser, TOKEN_RETURN)) {
        return parse_return_statement(parser);
    }
    return parse_expression(parser);
}

static AstNode* parse_return_statement(Parser* parser) {
    AstNode* node = malloc(sizeof(AstNode));
    node->type = NODE_RETURN;

    if (match_parser(parser, TOKEN_LEFT_PAREN)) {
        // Multi-value return
        struct AstNode** values = malloc(sizeof(AstNode*) * 8);
        int count = 0;
        int capacity = 8;

        do {
            if (count == capacity) {
                capacity *= 2;
                values = realloc(values, sizeof(AstNode*) * capacity);
            }

            values[count++] = parse_expression(parser);

        } while (match_parser(parser, TOKEN_COMMA));

        if (!match_parser(parser, TOKEN_RIGHT_PAREN)) {
            error(parser, "Expected ')' after return values");
            return NULL;
        }

        node->value.return_stmt.return_value = malloc(sizeof(AstNode));
        node->value.return_stmt.return_value->type = NODE_TUPLE;
        node->value.return_stmt.return_value->value.tuple.values = values;
        node->value.return_stmt.return_value->value.tuple.value_count = count;
    } else {
        // Single-value return
        node->value.return_stmt.return_value = parse_expression(parser);
    }

    return node;
}

static void free_ast(AstNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_FUNCTION:
            free(node->value.function.name);
            for (int i = 0; i < node->value.function.param_count; i++) {
                free_ast(node->value.function.parameters[i]);
            }
            free(node->value.function.parameters);
            free(node->value.function.body);
            free(node->value.function.return_types);
            break;
        case NODE_RETURN:
            free_ast(node->value.return_stmt.return_value);
            break;
        case NODE_BINARY_OP:
            free_ast(node->value.binary_op.left);
            free_ast(node->value.binary_op.right);
            break;
        case NODE_VARIABLE:
            free(node->value.variable.name);
            free_ast(node->value.variable.init_value);
            break;
        case NODE_TUPLE:
            for (int i = 0; i < node->value.tuple.value_count; i++) {
                free_ast(node->value.tuple.values[i]);
            }
            free(node->value.tuple.values);
            break;
    }
    free(node);
}

static void run_test(const char* source, const char* test_name) {
    printf("\nRunning test: %s\n", test_name);
    printf("Source: %s\n", source);

    Lexer lexer;
    init_lexer(&lexer, source);

    Parser parser;
    init_parser(&parser, &lexer);

    AstNode* ast = parse_function(&parser);
    if (ast != NULL) {
        printf("Successfully parsed function!\n");
        // TODO: Add AST printer
        free_ast(ast);
    } else {
        printf("Failed to parse function!\n");
    }
}

int main(void) {
    const char* test1 = "f main() -> null:\n    return null";
    run_test(test1, "Simple function with null return");

    const char* test2 = "f div(a: int, b: int) -> (int, error):\n    return (a / b, null)";
    run_test(test2, "Function with multiple return values");

    const char* test3 =
            "f div(a: int, b: int) -> (int, error):\n"
            "    if b == 0:\n"
            "        return (0, error(\"Division by zero\"))\n"
            "    return (a / b, null)";
    run_test(test3, "Function with error handling");

    const char* test4 = "f process() -> (i32, str, error):\n    return (42, \"success\", null)";
    run_test(test4, "Function with different return types");

    const char* test5 = "f get_age() -> u8:\n    return 25";
    run_test(test5, "Function with simple number return");

    const char* test6 = "f get_name() -> str:\n    return \"John\"";
    run_test(test6, "Function with string return");

    return 0;
}
