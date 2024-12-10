#include "base.h"
#include "stb_ds.h"
#include "parser.h"

#define Consume(t) \
if (!(tokens[pkg->i].type == (t))) {\
    pkg->err.isErr = true; \
    pkg->err.err = S("Expected " STR(t)); \
    return (ABase*)a; \
} \
pkg->i += 1;

ABase *Parse(Package *pkg, Token *tokens);

ABase *ParseString(Package *pkg, Token *tokens) {
    AExprLit *a = ArenaAlloc(&pkg->pa, sizeof(AExprLit));
    a->base.type = AST_EXPR_LIT;
    a->str = tokens[pkg->i].lit;
    pkg->i += 1;
    return (ABase*)a;
}

ABase *ParseCompDir(Package *pkg, Token *tokens) {
    AStmtCompDir *a = ArenaAlloc(&pkg->pa, sizeof(AStmtCompDir));
    a->base.type = AST_STMT_COMP_DIR;
    a->directive = tokens[pkg->i].lit;
    pkg->i++;
    a->arg = Parse(pkg, tokens);
    if (pkg->err.isErr) return (ABase*)a;
    Consume(';');
    return (ABase*)a;
}

ABase *ParseIdent(Package *pkg, Token *tokens) {
   if (pkg->i+1 >= arrlen(tokens)) {
        pkg->err.isErr = true;
        pkg->err.err = S("Unexpected identifier");
        return NULL;
    }
   
    switch ((int)tokens[pkg->i+1].type) {
    case ':': {
        if (pkg->i+2 >= arrlen(tokens)) {
            pkg->err.isErr = true;
            pkg->err.err = S("Expected more tokens");
            return NULL;
        }
        
        AStmtDecl *decl = ArenaAlloc(&pkg->pa, sizeof(AStmtDecl));
        decl->base.type = AST_STMT_DECL;
        decl->ident = tokens[pkg->i].lit;
        pkg->i += 2; // Consume both the literal, and the ':'
        
        switch ((int)tokens[pkg->i].type) {
        case TOKEN_IDENT: { // TODO: Type...
            break;
        }
        case ':': {
            AStmtConstAssign *a = ArenaAlloc(&pkg->pa, sizeof(AStmtConstAssign));
            a->base.type = AST_STMT_CONST_ASSIGN;
            a->target = (ABase*)decl;
            pkg->i += 1; // Consume the second ':'
            a->value = Parse(pkg, tokens);
            if ( a->value->type != AST_EXPR_FUNC ) {
                Consume(';');
            }
            return a;
            break;
        }
        }
        
        break;
    }
    }
    return NULL;
}

ABase *Parse(Package *pkg, Token *tokens)
{
    if (pkg->i >= arrlen(tokens)) {
        pkg->err.isErr = true;
        pkg->err.err = S("Expected more tokens");
        return NULL;
    }
    
    switch (tokens[pkg->i].type) {
        case TOKEN_COMP_DIR: {
            return ParseCompDir(pkg, tokens);
            break;
        }
        case TOKEN_STRING: {
            return ParseString(pkg, tokens);
            break;
        }
        case TOKEN_IDENT: {
            return ParseIdent(pkg, tokens);
            break;
        }
        default: {        
            pkg->err.isErr = true;
            pkg->err.err = S("Unexpected token");
            return NULL;
            break;
        }
    }
    
    return NULL;
}

bool ParsePackage(Package *pkg, Token *tokens)
{
    pkg->module.base.type = AST_SCOPE;
    for (; pkg->i < arrlen(tokens); ++pkg->i) {
        ABase *a = Parse(pkg, tokens);
        if (pkg->err.isErr) {
            return true;
        }
        arrpush(pkg->module.scope, a);
    }
}

#undef Consume