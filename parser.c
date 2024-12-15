#include "base.h"
#include "stb_ds.h"
#include "parser.h"

static void error(Parser *p, string msg)
{
    p->is_err = true;
    p->err = msg;
}

static Token peek(Parser *p)
{
    return p->tokens[p->curr_token];
}

static Token get(Parser *p)
{
    Token t = peek(p);
    p->curr_token += 1;
    return t;
}

static bool consume(Parser *p, TokenType typ)
{
    Token t = get(p);
    bool valid = t.type == typ;
    if (!valid) {
        p->is_err = true;
        p->err = S("Expected token not found.");
    }
    return valid;
}

ANode *parse_expr(Parser *p)
{
    AExpr *expr = NULL;
    switch (peek(p).type) {
    case TOKEN_STRING: {
        ALit *a = arena_alloc(&p->node_arena, sizeof(ALit));
        a->base.base.kind = NODE_LIT;

        a->kind = LIT_STRING;
        a->as.STRING = get(p).lit;

        expr = (AExpr*)a;
    } break;
    default: error(p, S("Unexpected token")); return (ANode*)expr;
    }

    while (peek(p).type == TOKEN_LABEL) {
        arrpush(expr->labels, get(p).lit);
    }

    return (ANode*)expr;
}

ANode *parse_comp_dir(Parser *p)
{
    ACompDir *a = arena_alloc(&p->node_arena, sizeof(ACompDir));
    memset(a, 0, sizeof(ACompDir));
    a->base.base.kind = NODE_COMP_DIR;

    Token t = get(p);

    a->directive = t.lit;

    while (peek(p).type != ';') {
        arrpush(a->args, parse_expr(p));
    }

    consume(p, ';');

    return (ANode*)a;
}

ANode *parse_stmt(Parser *p)
{
    AStmt *a = NULL;
    switch (peek(p).type) {
    case TOKEN_COMP_DIR: {
        a = (AStmt*)parse_comp_dir(p);
        if (!a) return (ANode*)a; // On error we fail immediately...
    } break;
    default: error(p, S("Unexpected token")); (ANode*)a;
    }
    while (peek(p).type == TOKEN_LABEL) {
        arrpush(a->labels, get(p).lit);
    }
    return (ANode*)a;
}

ANode *parse_scope(Parser *p)
{
    AScope *scope = arena_alloc(&p->node_arena, sizeof(AScope));
    memset(scope, 0, sizeof(AScope));
    scope->base.kind = NODE_SCOPE;

    ANode *a = NULL;
    while (peek(p).type != '}' || p->curr_token == arrlen(p->tokens)) {
        a = parse_stmt(p);
        arrpush(scope->stmts, a);
        if (p->is_err) break;
    }

    return scope;
}

ANode *parser_parse_tokens(Parser *p)
{
    return parse_scope(p);
}

Parser parser_from_lexer(Lexer *lex)
{
    return (Parser){ .tokens=lex->tokens };
}

void parser_free(Parser *p)
{

}

void print_ast(ANode *a)
{
    if (!a) return;
    switch (a->kind) {
    case NODE_SCOPE: {
        AScope *scope = (AScope*)a;
        printf("{\n");
        for (int i = 0; i < arrlen(scope->stmts); ++i) {
            print_ast(scope->stmts[i]);
        }
        printf("}\n");
    } break;
    case NODE_COMP_DIR: {
        ACompDir *cd = (ACompDir*)a;
        printf("#%.*s\n", cd->directive.len, cd->directive.data);
        for (int i = 0; i < arrlen(cd->args); ++i) {
            print_ast(cd->args[i]);
        }
        printf(";\n");
    } break;
    case NODE_LIT: {
        ALit *lit = (ALit*)a;
        switch (lit->kind) {
        case LIT_STRING: {
            printf("LIT: %.*s\n", lit->as.STRING.len, lit->as.STRING.data);
        } break;
        }
    } break;
    }
}


#undef Consume
