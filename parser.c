#include "base.h"
#include "stb_ds.h"
#include "parser.h"

#define Consume(t) \
if (!(p->tokens[p->i].type == (t))) {\
    p->err.isErr = true; \
    p->err.err = S("Expected " STR(t)); \
    return (ABase*)a; \
} \
p->i += 1;

ABase *Parse(Parser *p);
AScope *ParseScope(Parser *p);

ABase *ParseString(Parser *p) {
    AExprLit *a = ArenaAlloc(&p->pa, sizeof(AExprLit));
    a->base.type = AST_EXPR_LIT;
    a->str = p->tokens[p->i].lit;
    p->i += 1;
    return (ABase*)a;
}

ABase *ParseCompDir(Parser *p) {
    AStmtCompDir *a = ArenaAlloc(&p->pa, sizeof(AStmtCompDir));
    a->base.type = AST_STMT_COMP_DIR;
    a->directive = p->tokens[p->i].lit;
    p->i++;
    a->arg = Parse(p);
    if (p->err.isErr) return (ABase*)a;
    Consume(';');
    return (ABase*)a;
}

ABase *ParseIdent(Parser *p) {
   if (p->i+1 >= arrlen(p->tokens)) {
        p->err.isErr = true;
        p->err.err = S("Unexpected identifier");
        return NULL;
    }

    switch ((int)p->tokens[p->i+1].type) {
    case ':': {
        if (p->i+2 >= arrlen(p->tokens)) {
            p->err.isErr = true;
            p->err.err = S("Expected more tokens");
            return NULL;
        }

        AStmtDecl *decl = ArenaAlloc(&p->pa, sizeof(AStmtDecl));
        decl->base.type = AST_STMT_DECL;
        decl->ident = p->tokens[p->i].lit;
        p->i += 2; // Consume both the literal, and the ':'

        switch ((int)p->tokens[p->i].type) {
        case TOKEN_IDENT: { // TODO: Type...
            break;
        }
        case ':': {
            AStmtConstAssign *a = ArenaAlloc(&p->pa, sizeof(AStmtConstAssign));
            a->base.type = AST_STMT_CONST_ASSIGN;
            a->target = (ABase*)decl;
            p->i += 1; // Consume the second ':'
            a->value = Parse(p);
            if ( a->value->type != AST_EXPR_FUNC ) {
                Consume(';');
            }
            return (ABase*)a;
            break;
        }
        }

        break;
    }
    }
    return NULL;
}

ABase *Parse(Parser *p)
{
    if (p->i >= arrlen(p->tokens)) {
        p->err.isErr = true;
        p->err.err = S("Expected more tokens");
        return NULL;
    }

    switch (p->tokens[p->i].type) {
        case TOKEN_COMP_DIR: {
            return ParseCompDir(p);
            break;
        }
        case TOKEN_STRING: {
            return ParseString(p);
            break;
        }
        case TOKEN_IDENT: {
            return ParseIdent(p);
            break;
        }
        default: {
            p->err.isErr = true;
            p->err.err = S("Unexpected token");
            return NULL;
            break;
        }
    }

    return NULL;
}

Parser ParserFromLexer(Lexer *lex)
{
    return (Parser){
        .tokens = lex->tokens,
    };
}

bool ParserParseTokens(Parser *p)
{
    p->glob.base.type = AST_SCOPE;
    for (; p->i < arrlen(p->tokens); ++p->i) {
        ABase *a = Parse(p);
        if (p->err.isErr) {
            return true;
        }
        arrpush(p->glob.scope, a);
    }
}

void ParserFree(Parser *p)
{
    // Cleanup...
}

#undef Consume