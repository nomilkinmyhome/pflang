#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

    // Other tokens
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_COLON,         // :
    TOKEN_ARROW,         // ->
    TOKEN_EQUALS,        // =
    TOKEN_EOF,
    TOKEN_LEFT_PAREN,    // (
    TOKEN_RIGHT_PAREN,   // )
    TOKEN_LEFT_BRACE,    // {
    TOKEN_RIGHT_BRACE,   // }
    TOKEN_COMMA,         // ,
    TOKEN_DOT,           // .
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
    NODE_PARAMETER,
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

        // Unary operator
        struct {
            TokenType operator;
            struct AstNode* operand;
        } unary_op;
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
        case 'r': return check_keyword(lexer, 1, 5, "eturn", TOKEN_RETURN);
        case 'i':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'f': return check_keyword(lexer, 1, 1, "f", TOKEN_IF);
                    case 'n': return check_keyword(lexer, 1, 2, "nt", TOKEN_I32);
                    case '3': return check_keyword(lexer, 1, 2, "32", TOKEN_I32);
                    case '1': return check_keyword(lexer, 1, 2, "16", TOKEN_I16);
                    case '6': return check_keyword(lexer, 1, 2, "64", TOKEN_I64);
                    case '8': return check_keyword(lexer, 1, 1, "8", TOKEN_I8);
                }
            }
            break;
        case 'u':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case '8': return check_keyword(lexer, 1, 1, "8", TOKEN_U8);
                    case '1': return check_keyword(lexer, 1, 2, "16", TOKEN_U16);
                    case '3': return check_keyword(lexer, 1, 2, "32", TOKEN_U32);
                    case '6': return check_keyword(lexer, 1, 2, "64", TOKEN_U64);
                }
            }
            break;
        case 'n': return check_keyword(lexer, 1, 3, "ull", TOKEN_NULL);
        case 's': return check_keyword(lexer, 1, 2, "tr", TOKEN_STR);
        case 'b': return check_keyword(lexer, 1, 3, "ool", TOKEN_BOOL);
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
        case 'f':
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case '3': return check_keyword(lexer, 1, 2, "32", TOKEN_F32);
                    case '6': return check_keyword(lexer, 1, 2, "64", TOKEN_F64);
                }
            } else {
                return check_keyword(lexer, 1, 0, "", TOKEN_FUNCTION);
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
        case '+':
            if (match_lexer(lexer, '+')) return make_token(lexer, TOKEN_INCREMENT);
            return make_token(lexer, TOKEN_PLUS);
        case '*': return make_token(lexer, TOKEN_MULTIPLY);
        case '/': return make_token(lexer, TOKEN_DIVIDE);
        case '%': return make_token(lexer, TOKEN_MODULO);
        case '=':
            if (match_lexer(lexer, '=')) return make_token(lexer, TOKEN_EQUALS);
            return make_token(lexer, TOKEN_EQUALS);
        case '>':
            if (match_lexer(lexer, '=')) return make_token(lexer, TOKEN_GREATER_EQUAL);
            return make_token(lexer, TOKEN_GREATER);
        case '<':
            if (match_lexer(lexer, '=')) return make_token(lexer, TOKEN_LESS_EQUAL);
            return make_token(lexer, TOKEN_LESS);
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

static void error(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;

    fprintf(stderr, "[line %d] Error at '%s': %s\n",
            parser->previous.line,
            parser->previous.lexeme,
            message);
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
        case NODE_PARAMETER:
            free(node->value.parameter.name);
            break;
        case NODE_TUPLE:
            for (int i = 0; i < node->value.tuple.value_count; i++) {
                free_ast(node->value.tuple.values[i]);
            }
            free(node->value.tuple.values);
            break;
        case NODE_UNARY_OP:
            free_ast(node->value.unary_op.operand);
            break;
        case NODE_LITERAL:
            free(node->value.literal.value);
            break;
    }
    free(node);
}

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

static AstNode* make_binary_op(AstNode* left, TokenType operator, AstNode* right) {
    AstNode* node = malloc(sizeof(AstNode));
    node->type = NODE_BINARY_OP;
    node->value.binary_op.left = left;
    node->value.binary_op.right = right;
    node->value.binary_op.operator = operator;
    return node;
}

static AstNode* parse_literal(Parser* parser) {
    AstNode* node = malloc(sizeof(AstNode));
    node->type = NODE_LITERAL;

    switch (parser->current.type) {
        case TOKEN_NUMBER:
            node->value.literal.value = strdup(parser->current.lexeme);
            node->value.literal.type = TYPE_I32;
            advance_parser(parser);
            break;
        case TOKEN_STRING:
            node->value.literal.value = strdup(parser->current.lexeme);
            node->value.literal.type = TYPE_STR;
            advance_parser(parser);
            break;
        case TOKEN_NULL:
            node->value.literal.value = strdup("null");
            node->value.literal.type = TYPE_NULL;
            advance_parser(parser);
            break;
        case TOKEN_IDENTIFIER:
            node->value.literal.value = strdup(parser->current.lexeme);
            node->value.literal.type = TYPE_I32;
            advance_parser(parser);
            break;
        default:
            free(node);
            return NULL;
    }

    return node;
}

static AstNode* parse_primary(Parser* parser) {
    if (match_parser(parser, TOKEN_LEFT_PAREN)) {
        AstNode* expr = parse_expression(parser);
        if (!match_parser(parser, TOKEN_RIGHT_PAREN)) {
            error(parser, "Expected ')' after expression");
            return NULL;
        }
        return expr;
    }

    if (match_parser(parser, TOKEN_NUMBER) ||
        match_parser(parser, TOKEN_STRING) ||
        match_parser(parser, TOKEN_NULL) ||
        match_parser(parser, TOKEN_IDENTIFIER)) {
        AstNode* node = malloc(sizeof(AstNode));
        node->type = NODE_LITERAL;
        node->value.literal.value = strdup(parser->previous.lexeme);
        node->value.literal.type = TYPE_I32;
        return node;
    }

    error(parser, "Expected expression");
    return NULL;
}

static AstNode* parse_unary(Parser* parser) {
    if (match_parser(parser, TOKEN_MINUS) ||
        match_parser(parser, TOKEN_PLUS) ||
        match_parser(parser, TOKEN_INCREMENT) ||
        match_parser(parser, TOKEN_DECREMENT)) {
        TokenType operator = parser->previous.type;
        AstNode* right = parse_unary(parser);

        AstNode* node = malloc(sizeof(AstNode));
        node->type = NODE_UNARY_OP;
        node->value.unary_op.operator = operator;
        node->value.unary_op.operand = right;
        return node;
    }

    return parse_primary(parser);
}

static AstNode* parse_factor(Parser* parser) {
    AstNode* left = parse_unary(parser);

    while (match_parser(parser, TOKEN_MULTIPLY) ||
           match_parser(parser, TOKEN_DIVIDE) ||
           match_parser(parser, TOKEN_MODULO)) {
        TokenType operator = parser->previous.type;
        AstNode* right = parse_unary(parser);
        left = make_binary_op(left, operator, right);
    }

    return left;
}

static AstNode* parse_term(Parser* parser) {
    AstNode* left = parse_factor(parser);

    while (match_parser(parser, TOKEN_PLUS) ||
           match_parser(parser, TOKEN_MINUS)) {
        TokenType operator = parser->previous.type;
        AstNode* right = parse_factor(parser);
        left = make_binary_op(left, operator, right);
    }

    return left;
}

static AstNode* parse_equality(Parser* parser) {
    AstNode* expr = parse_comparison(parser);

    while (match_parser(parser, TOKEN_EQUALS)) {
        TokenType operator = parser->previous.type;
        AstNode* right = parse_comparison(parser);
        expr = make_binary_op(expr, operator, right);
    }

    return expr;
}

static AstNode* parse_comparison(Parser* parser) {
    AstNode* expr = parse_term(parser);

    while (match_parser(parser, TOKEN_GREATER) ||
           match_parser(parser, TOKEN_GREATER_EQUAL) ||
           match_parser(parser, TOKEN_LESS) ||
           match_parser(parser, TOKEN_LESS_EQUAL)) {
        TokenType operator = parser->previous.type;
        AstNode* right = parse_term(parser);
        expr = make_binary_op(expr, operator, right);
    }

    return expr;
}

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
        int capacity = 8;
        node->value.function.parameters = malloc(sizeof(AstNode*) * capacity);

        do {
            if (node->value.function.param_count == capacity) {
                capacity *= 2;
                node->value.function.parameters = realloc(
                        node->value.function.parameters,
                        sizeof(AstNode*) * capacity
                );
            }

            AstNode* param = parse_parameter(parser);
            if (param == NULL) {
                for (int i = 0; i < node->value.function.param_count; i++) {
                    free_ast(node->value.function.parameters[i]);
                }
                free(node->value.function.parameters);
                free_ast(node);
                return NULL;
            }

            node->value.function.parameters[node->value.function.param_count++] = param;

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
            } else if (match_parser(parser, TOKEN_IDENTIFIER) && strcmp(parser->previous.lexeme, "int") == 0) {
                node->value.function.return_types[node->value.function.return_type_count++] = TYPE_I32;
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
        } else if (match_parser(parser, TOKEN_IDENTIFIER) && strcmp(parser->previous.lexeme, "int") == 0) {
            node->value.function.return_types[0] = TYPE_I32;
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
    return parse_equality(parser);
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
            free(values);
            free(node);
            return NULL;
        }

        if (count > 1) {
            node->value.return_stmt.return_value = malloc(sizeof(AstNode));
            node->value.return_stmt.return_value->type = NODE_TUPLE;
            node->value.return_stmt.return_value->value.tuple.values = values;
            node->value.return_stmt.return_value->value.tuple.value_count = count;
        } else {
            node->value.return_stmt.return_value = values[0];
            free(values);
        }
    } else {
        // Single-value return
        node->value.return_stmt.return_value = parse_expression(parser);
    }

    return node;
}

static AstNode* parse_parameter(Parser* parser) {
    if (!match_parser(parser, TOKEN_IDENTIFIER)) {
        error(parser, "Expected parameter name");
        return NULL;
    }

    AstNode* param = malloc(sizeof(AstNode));
    param->type = NODE_PARAMETER;
    param->value.parameter.name = strdup(parser->previous.lexeme);

    if (!match_parser(parser, TOKEN_COLON)) {
        error(parser, "Expected ':' after parameter name");
        free_ast(param);
        return NULL;
    }

    if (match_parser(parser, TOKEN_U8)) {
        param->value.parameter.type = TYPE_U8;
    } else if (match_parser(parser, TOKEN_U16)) {
        param->value.parameter.type = TYPE_U16;
    } else if (match_parser(parser, TOKEN_U32)) {
        param->value.parameter.type = TYPE_U32;
    } else if (match_parser(parser, TOKEN_U64)) {
        param->value.parameter.type = TYPE_U64;
    } else if (match_parser(parser, TOKEN_I8)) {
        param->value.parameter.type = TYPE_I8;
    } else if (match_parser(parser, TOKEN_I16)) {
        param->value.parameter.type = TYPE_I16;
    } else if (match_parser(parser, TOKEN_I32)) {
        param->value.parameter.type = TYPE_I32;
    } else if (match_parser(parser, TOKEN_I64)) {
        param->value.parameter.type = TYPE_I64;
    } else if (match_parser(parser, TOKEN_F32)) {
        param->value.parameter.type = TYPE_F32;
    } else if (match_parser(parser, TOKEN_F64)) {
        param->value.parameter.type = TYPE_F64;
    } else if (match_parser(parser, TOKEN_STR)) {
        param->value.parameter.type = TYPE_STR;
    } else if (match_parser(parser, TOKEN_BOOL)) {
        param->value.parameter.type = TYPE_BOOL;
    } else if (match_parser(parser, TOKEN_NULL)) {
        param->value.parameter.type = TYPE_NULL;
    } else if (match_parser(parser, TOKEN_IDENTIFIER) && strcmp(parser->previous.lexeme, "int") == 0) {
        param->value.parameter.type = TYPE_I32;
    } else {
        error(parser, "Expected parameter type");
        free_ast(param);
        return NULL;
    }

    return param;
}

static const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_FUNCTION: return "FUNCTION";
        case NODE_BLOCK: return "BLOCK";
        case NODE_RETURN: return "RETURN";
        case NODE_IF: return "IF";
        case NODE_WHILE: return "WHILE";
        case NODE_FOR: return "FOR";
        case NODE_BINARY_OP: return "BINARY_OP";
        case NODE_UNARY_OP: return "UNARY_OP";
        case NODE_VARIABLE: return "VARIABLE";
        case NODE_PARAMETER: return "PARAMETER";
        case NODE_LITERAL: return "LITERAL";
        case NODE_TUPLE: return "TUPLE";
        default: return "UNKNOWN";
    }
}

static const char* data_type_to_string(DataType type) {
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

static void print_ast_node(AstNode* node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");

    printf("%s", node_type_to_string(node->type));

    switch (node->type) {
        case NODE_FUNCTION:
            printf(" '%s' (", node->value.function.name);

            for (int i = 0; i < node->value.function.param_count; i++) {
                if (i > 0) printf(", ");
                AstNode* param = node->value.function.parameters[i];
                printf("%s: %s",
                       param->value.parameter.name,
                       data_type_to_string(param->value.parameter.type));
            }
            printf(") returns (");

            for (int i = 0; i < node->value.function.return_type_count; i++) {
                if (i > 0) printf(", ");
                printf("%s", data_type_to_string(node->value.function.return_types[i]));
            }
            printf(")\n");
            print_ast_node(node->value.function.body, depth + 1);
            break;
        case NODE_RETURN:
            printf("\n");
            print_ast_node(node->value.return_stmt.return_value, depth + 1);
            break;
        case NODE_TUPLE:
            printf(" with %d values:\n", node->value.tuple.value_count);
            for (int i = 0; i < node->value.tuple.value_count; i++) {
                print_ast_node(node->value.tuple.values[i], depth + 1);
            }
            break;
        case NODE_LITERAL:
            printf(" = %s\n", node->value.literal.value);
            break;
        case NODE_BINARY_OP:
            printf(" operator: ");
            switch (node->value.binary_op.operator) {
                case TOKEN_PLUS: printf("+"); break;
                case TOKEN_MINUS: printf("-"); break;
                case TOKEN_MULTIPLY: printf("*"); break;
                case TOKEN_DIVIDE: printf("/"); break;
                case TOKEN_MODULO: printf("%%"); break;
                case TOKEN_GREATER: printf(">"); break;
                case TOKEN_GREATER_EQUAL: printf(">="); break;
                case TOKEN_LESS: printf("<"); break;
                case TOKEN_LESS_EQUAL: printf("<="); break;
                case TOKEN_EQUALS: printf("=="); break;
                default: printf("unknown(%d)", node->value.binary_op.operator);
            }
            printf("\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Left:\n");
            print_ast_node(node->value.binary_op.left, depth + 2);
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("Right:\n");
            print_ast_node(node->value.binary_op.right, depth + 2);
            break;
        case NODE_UNARY_OP:
            printf(" operator: ");
            switch (node->value.unary_op.operator) {
                case TOKEN_MINUS: printf("-"); break;
                case TOKEN_PLUS: printf("+"); break;
                case TOKEN_INCREMENT: printf("++"); break;
                case TOKEN_DECREMENT: printf("--"); break;
                default: printf("unknown");
            }
            printf("\n");
            print_ast_node(node->value.unary_op.operand, depth + 1);
            break;
        default:
            printf("\n");
    }
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
        printf("AST:\n");
        print_ast_node(ast, 0);
        free_ast(ast);
    } else {
        printf("Failed to parse function!\n");
    }
    printf("\n");
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

    const char* test7 = "f calc() -> i32:\n    return 2 + 3 * 4";
    run_test(test7, "Function with arithmetic expression");

    const char* test8 = "f compare(x: int, y: int) -> bool:\n    return x > y + 5";
    run_test(test8, "Function with comparison expression");

    const char* test9 = "f complex() -> i32:\n    return (2 + 3) * (4 - 1)";
    run_test(test9, "Function with parenthesized expression");

    const char* test10 = "f unary() -> i32:\n    return -5 + +3";
    run_test(test10, "Function with unary operators");

    const char* test11 = "f mixed() -> bool:\n    return 10 * 5 >= 45 + 5";
    run_test(test11, "Function with mixed operators");

    const char* test12 = "f increment(x: int) -> int:\n    return ++x";
    run_test(test12, "Function with increment operator");

    const char* test13 = "f priority() -> i32:\n    return 1 + 2 * 3 - 4 / 2";
    run_test(test13, "Function testing operator precedence");

    const char* test14 = "f complex_expr(a: int, b: int) -> bool:\n    return (a + b) * 2 >= 10 - 5";
    run_test(test14, "Function with complex expression");

    return 0;
}
