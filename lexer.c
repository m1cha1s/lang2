#include "lexer.h"
#include "stb_ds.h"

Lexer LexerFromSrc(String src)
{
    Lexer lex = {0};
    
    lex.line = 0;
    lex.col = 0;
    
    lex.src = src;
    lex.head = 0;
    
    lex.tokens = NULL;
    
    return lex;
}

LexerError LexerLexSrc(Lexer *lex)
{
    LexerError err = {0};
    
    
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
                
                if (IsWhitespace(lex->src.data[lex->head])) {
                    if (lex->src.data[lex->head] == '\n') {
                        lex->line += 1;
                        lex->col = 0;
                    }
                        
                    break;
                }
            }
            
            t.lit = StringViewFromBytes(&lex->src.data[start+1], lex->head-start-1);
            t.line = lex->line;
            t.col = lex->col;
            t.type = TOKEN_COMP_DIR;
            
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
                
                printf("aaa\n");
            }
            
            lex->head += 1;
            lex->col += 1;
            
            t.lit = StringViewFromBytes(&lex->src.data[start+1], lex->head-2-start);
            t.line = lex->line;
            t.col = lex->col;
            t.type = TOKEN_STRING;
            
            arrpush(lex->tokens, t);
            
            break;
        }
        case ';': {
            
            t.line = lex->line;
            t.col = lex->col;
            t.type = ';';
            
            arrpush(lex->tokens, t);
            
            lex->head += 1;
            lex->col += 1;
            
            break;
        }
        default: {
            err.isErr = true;
            err.err = S("Unexpected character");
            return err;
            break;
        }
        }
    }
}