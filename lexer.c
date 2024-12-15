#include "lexer.h"
#include "stb_ds.h"

Lexer lexer_from_src(string src)
{
    Lexer lex = {0};

    lex.line = 0;
    lex.col = 0;

    lex.src = src;
    lex.head = 0;

    lex.tokens = NULL;

    return lex;
}

LexerError lexer_lex_src(Lexer *lex)
{
    LexerError err = {0};

    #define SingleCharToken(c) \
    case c: { \
            t.line = lex->line; \
            t.col = lex->col; \
            t.type = c; \
            arrpush(lex->tokens, t); \
            lex->head += 1; \
            lex->col += 1; \
            break; \
    }

    while (lex->head < lex->src.len) {
        Token t = {0};

        switch (lex->src.data[lex->head]) {
        case '\n': {
            lex->line += 1;
            lex->col = 0;
        }
        case ' ':
        case '\t': {
            lex->col += 1;
            lex->head += 1;
            continue;
        }
        case '#': {
            usize start = lex->head;
            while (1) {
                lex->head += 1;
                lex->col += 1;

                if (!(IsAlpha(lex->src.data[lex->head]) || IsNum(lex->src.data[lex->head]) || lex->src.data[lex->head] == '_')) {
                    if (lex->src.data[lex->head] == '\n') {
                        lex->line += 1;
                        lex->col = 0;
                    }

                    break;
                }
            }

            t.lit = string_view_from_bytes(&lex->src.data[start+1], lex->head-start-1);
            t.line = lex->line;
            t.col = lex->col;
            t.type = TOKEN_COMP_DIR;

            arrpush(lex->tokens, t);

            break;
        }
        case '@': {
            usize start = lex->head;
            while (1) {
                lex->head += 1;
                lex->col += 1;

                if (!(IsAlpha(lex->src.data[lex->head]) || IsNum(lex->src.data[lex->head]) || lex->src.data[lex->head] == '_')) {
                    if (lex->src.data[lex->head] == '\n') {
                        lex->line += 1;
                        lex->col = 0;
                    }

                    break;
                }
            }

            t.lit = string_view_from_bytes(&lex->src.data[start+1], lex->head-start-1);
            t.line = lex->line;
            t.col = lex->col;
            t.type = TOKEN_LABEL;

            arrpush(lex->tokens, t);

            break;
        }
        case '"': {
            usize start = lex->head;
            while (1) {
                lex->head += 1;
                lex->col += 1;

                if (lex->src.data[lex->head] == '\n') {
                    lex->col = 0;
                    lex->line += 1;
                    if (lex->src.data[lex->head-1] != '\\') {
                        err.isErr = true;
                        err.err = S("Expected '\"'");
                        return err;
                    }
                }

                if (lex->src.data[lex->head] == '"') {
                    break;
                }
            }

            lex->head += 1;
            lex->col += 1;

            t.lit = string_view_from_bytes(&lex->src.data[start+1], lex->head-2-start);
            t.line = lex->line;
            t.col = lex->col;
            t.type = TOKEN_STRING;

            arrpush(lex->tokens, t);

            break;
        }
        SingleCharToken(';')
        SingleCharToken(':')
        SingleCharToken('(')
        SingleCharToken(')')
        SingleCharToken('{')
        SingleCharToken('}')
        SingleCharToken('=')
        SingleCharToken('+')
        case '-': {
            if (lex->src.data[lex->head+1] == '>') {
                t.line = lex->line;
                t.col = lex->col;
                t.type = TOKEN_ARROW;

                arrpush(lex->tokens, t);

                lex->head += 2;
                lex->col += 2;
            }
            else {
                t.line = lex->line;
                t.col = lex->col;
                t.type = '-';

                arrpush(lex->tokens, t);

                lex->head += 1;
                lex->col += 1;
            }

            break;
        }
        SingleCharToken('*')
        SingleCharToken('/')
        SingleCharToken(',')
        default: {
            if (IsAlpha(lex->src.data[lex->head])) {
                usize start = lex->head;
                while (1) {
                    lex->head += 1;
                    lex->col += 1;

                    if (!(IsAlpha(lex->src.data[lex->head]) || IsNum(lex->src.data[lex->head]) || lex->src.data[lex->head] == '_')) {
                        if (lex->src.data[lex->head] == '\n') {
                            lex->line += 1;
                            lex->col = 0;
                        }

                        break;
                    }
                }

                t.lit = string_view_from_bytes(&lex->src.data[start], lex->head-start);
                t.line = lex->line;
                t.col = lex->col;
                t.type = TOKEN_IDENT;

                arrpush(lex->tokens, t);

                break;
            }
            if (IsNum(lex->src.data[lex->head])) {
                usize start = lex->head;
                while (1) {
                    lex->head += 1;
                    lex->col += 1;

                    if (!(lex->src.data[lex->head]=='.' || IsNum(lex->src.data[lex->head]))) {
                        if (lex->src.data[lex->head] == '\n') {
                            lex->line += 1;
                            lex->col = 0;
                        }

                        break;
                    }
                }

                t.lit = string_view_from_bytes(&lex->src.data[start], lex->head-start);
                t.line = lex->line;
                t.col = lex->col;
                t.type = TOKEN_NUMBER;

                arrpush(lex->tokens, t);

                break;
            }

            err.isErr = true;
            err.err = S("Unexpected character");
            return err;
            break;
        }
        }
    }

    #undef SingleCharToken
    return err;
}

void lexer_free(Lexer *lex)
{
    arrfree(lex->tokens);
}