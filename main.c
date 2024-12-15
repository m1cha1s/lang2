/*

FEATURES:
- Function overloading
- Default function arguments
- Default struct arguments
- Metaprogramming (jai style)
- For loops, with iterators (jai style)
- Out of order top scope
- Nested function decls
- Simple type inference
- Proper string type
- Operator overloading
- Namespaces (package based (odin style))
- Enums (jai style)
- 64 and 32 bit target support (embedded as well)
- Fast compile times (jai style)
- C interop
- Code notes (jai style) (ex. for deprecation)

TODO:
- Error in lexer, wrong line, col info for tokens longer than a single char.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "stb_ds.h"

#include "base.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv)
{
    FILE *srcFile = fopen("text.l2", "rb");
    fseek(srcFile, 0L, SEEK_END);
    size_t srcFileSize = ftell(srcFile);
    rewind(srcFile);

    Arena a = {0};

    string src = {0};
    src.data = arena_alloc(&a, srcFileSize);
    fread(src.data, 1, srcFileSize, srcFile);
    src.len = srcFileSize;

    printf("--- src file ---\n%.*s\n----\n", (int)src.len, src.data);

    Lexer lex = lexer_from_src(src);

    LexerError lerr = lexer_lex_src(&lex);
    if (lerr.isErr)
        printf("%d:%d %.*s\n", (int)lex.line, (int)lex.col, (int)lerr.err.len, lerr.err.data);

    for (int i = 0; i < arrlen(lex.tokens); ++i) {
        if (lex.tokens[i].type >= TOKEN_IDENT)
            printf("  [%s] '%.*s'\n", TokenTypeNames[lex.tokens[i].type], (int)lex.tokens[i].lit.len, lex.tokens[i].lit.data);
        else
            printf("  [%c]\n", lex.tokens[i].type);
    }

    Parser p = parser_from_lexer(&lex);
    ANode *ast = parser_parse_tokens(&p);
    if (p.is_err) {
        printf("%d:%d %.*s\n", (int)lex.tokens[p.curr_token].line, (int)lex.tokens[p.curr_token].col, (int)p.err.len, p.err.data);
    }

    print_ast(ast);

    parser_free(&p);
    lexer_free(&lex);
}

#include "base.c"
#include "lexer.c"
#include "parser.c"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"