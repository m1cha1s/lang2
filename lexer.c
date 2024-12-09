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
        case '#': {
            usize start = lex->head;
            while (1) {
                lex->head += 1;
                lex->col += 1;
                
                if (IsWhitespace(lex->src.data[lex->head])) {
                    if (lex->src.data[lex->head] == '\n')
                        lex->line += 1;
                        
                    break;
                }
            }
            
            t.lit = StringViewFromBytes(&lex->src.data[start+1], lex->head-start);
            t.line = lex->line;
            t.col = lex->col;
            t.type = TOKEN_COMP_DIR;
            
            arrpush(lex->tokens, t);
            
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