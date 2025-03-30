#include "../include/lexer.h"

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
        advance_lexer(lexer);
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
            if (lexer->current - lexer->start == 1) {
                return TOKEN_FUNCTION;
            }
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case '3': return check_keyword(lexer, 1, 2, "32", TOKEN_F32);
                    case '6': return check_keyword(lexer, 1, 2, "64", TOKEN_F64);
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
        case '+':
            if (match_lexer(lexer, '+')) return make_token(lexer, TOKEN_INCREMENT);
            return make_token(lexer, TOKEN_PLUS);
        case '*': return make_token(lexer, TOKEN_MULTIPLY);
        case '/': return make_token(lexer, TOKEN_DIVIDE);
        case '%': return make_token(lexer, TOKEN_MODULO);
        case '=':
            if (match_lexer(lexer, '=')) return make_token(lexer, TOKEN_EQUALS);
            return make_token(lexer, TOKEN_ASSIGNMENT);
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
