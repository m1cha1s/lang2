#ifndef _LEXER_H
#define _LEXER_H

#include <stdint.h>
#include <stddef.h>

#include "base.h"

typedef enum TokenType {
    TOKEN_IDENT = 256,
    TOKEN_COMP_DIR,
    TOKEN_LABEL,
    TOKEN_STRING,
    TOKEN_NUMBER,

    TOKEN_ARROW,
    TOKEN_RETURN,
} TokenType;

const char *TokenTypeNames[] = {
    [TOKEN_IDENT] = "TOKEN_IDENT",
    [TOKEN_COMP_DIR] = "TOKEN_COMP_DIR",
    [TOKEN_LABEL] = "TOKEN_LABEL",
    [TOKEN_STRING] = "TOKEN_STRING",
    [TOKEN_NUMBER] = "TOKEN_NUMBER",

    [TOKEN_ARROW] = "->",
    [TOKEN_RETURN] = "return",
};

typedef struct Token {
    string lit;

    usize line, col; // This may be not necesary...

    TokenType type;
} Token;

typedef struct Lexer {
    string src;

    usize line, col;

    usize head;

    Token *tokens;
} Lexer;

typedef struct LexerError {
    bool isErr;
    string err;
} LexerError;

Lexer lexer_from_src(string src);
LexerError lexer_lex_src(Lexer *lex);

void lexer_free(Lexer *lex);

#endif