#include "../include/parser.h"

static void advance_parser(Parser* parser) {
    parser->previous = parser->current;
    parser->current = scan_token(parser->lexer);
}

void init_parser(Parser* parser, Lexer* lexer) {
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
    printf("Debug: Matching token type %d with current token type %d ('%s')\n",
           type, parser->current.type, parser->current.lexeme);
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
        match_parser(parser, TOKEN_IDENTIFIER)) {
        AstNode* node = malloc(sizeof(AstNode));
        node->type = NODE_LITERAL;
        node->value.literal.value = strdup(parser->previous.lexeme);
        node->value.literal.type = TYPE_I32;
        return node;
    }

    if (match_parser(parser, TOKEN_NULL)) {
        AstNode* node = malloc(sizeof(AstNode));
        node->type = NODE_LITERAL;
        node->value.literal.value = strdup("null");
        node->value.literal.type = TYPE_NULL;
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

AstNode* parse(Parser* parser) {
    printf("Debug: Starting parse\n");
    printf("Debug: First token type: %d, lexeme: '%s'\n",
           parser->current.type, parser->current.lexeme);

    AstNode* ast = parse_function(parser);
    if (ast == NULL) {
        printf("Debug: parse_function returned NULL\n");
    }

    if (!match_parser(parser, TOKEN_EOF)) {
        error(parser, "Expected end of file");
        return NULL;
    }

    if (parser->had_error) {
        printf("Debug: Parser had error\n");
        return NULL;
    }

    return ast;
}

bool had_parser_error(Parser* parser) {
    return parser->had_error;
}
