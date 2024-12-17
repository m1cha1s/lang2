#include "base.h"
#include "stb_ds.h"
#include "parser.h"

static void error(Parser *p, string msg)
{
    p->is_err = true;
    p->err = msg;
    p->err_token = p->curr_token;
}

Token tok_stub = {0};

static Token peek(Parser *p)
{
    if (p->curr_token >= arrlen(p->tokens)) {
        error(p, S("EOF"));
        return tok_stub;
    }
    return p->tokens[p->curr_token];
}

static Token peek2(Parser *p)
{
    if (p->curr_token+1 >= arrlen(p->tokens)) {
        error(p, S("EOF"));
        return tok_stub;
    }
    return p->tokens[p->curr_token+1];
}

static Token peek3(Parser *p)
{
    if (p->curr_token+2 >= arrlen(p->tokens)) {
        error(p, S("EOF"));
        return tok_stub;
    }
    return p->tokens[p->curr_token+2];
}

static Token get(Parser *p)
{
    if (p->curr_token >= arrlen(p->tokens)) {
        error(p, S("EOF"));
        return tok_stub;
    }
    Token t = peek(p);
    p->curr_token += 1;
    return t;
}

static bool consume(Parser *p, TokenType typ)
{
    Token t = get(p);
    bool valid = t.type == typ;
    if (!valid) {
        string tok_str = string_view_from_bytes(
            typ <= 255 ? (u8*)&typ : TokenTypeNames[typ], 
            typ <= 255 ? 1 : strlen(TokenTypeNames[typ])
        );
        error(p, string_concat(&p->node_arena, S("Expected "), tok_str));
    }
    return valid;
}

ANode *parse_decl(Parser *p);
ANode *parse_scope(Parser *p);
ANode *parse_expr(Parser *p);

ANode *parse_func(Parser *p)
{
    AFunc *a = arena_alloc(&p->node_arena, sizeof(AFunc));
    memset(a, 0, sizeof(AFunc));
    a->base.base.kind = NODE_FUNC;

    consume(p, '(');

    while (1) {
        printf("\t%c\n", peek(p).type);
        if (peek(p).type != ')') arrpush(a->args, parse_decl(p));
        if (p->is_err) return (ANode*)a;
        if (peek(p).type == ',') {
            consume(p, ',');
            if (p->is_err) return (ANode*)a;
            continue;
        }
        if (peek(p).type == ')') break;
        printf("aaa\n");
    }

    consume(p, ')');
    if (p->is_err) return (ANode*)a;

    if (peek(p).type == TOKEN_ARROW) {
        consume(p, TOKEN_ARROW);
        if (p->is_err) return (ANode*)a;

        while (1) {
            arrpush(a->rets, get(p).lit);
            if (peek(p).type == ',') {
                consume(p, ',');
                if (p->is_err) return (ANode*)a;
                continue;
            }
            break;
        }
    }

    if (peek(p).type == '{') {
        consume(p, '{');
        if (p->is_err) return (ANode*)a;

        a->body = parse_scope(p);
        
        consume(p, '}');
    }


    // if (p->is_err)

    return (ANode*)a;
}

ANode *parse_func_call(Parser *p)
{
    AFuncCall *fn = arena_alloc(&p->node_arena, sizeof(AFuncCall));
    memset(fn, 0, sizeof(AFuncCall));
    fn->base.base.kind = NODE_FUNC_CALL;
    
    fn->name = get(p).lit;
    
    consume(p, '(');
    
    while (1) {
        arrpush(fn->args, parse_expr(p));
        if (peek(p).type == ')') break;
        consume(p, ',');
    }
    
    consume(p, ')');
    
    return (ANode*)fn;
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
    case TOKEN_IDENT: {
        TokenType t = peek2(p).type;
        if (t == '(') {
            expr = (AExpr*)parse_func_call(p);
            break;
        } else
        if (t == '+' || t == '-' || t == '*' || t == '/' || t == '%') { // Binop
        
        } else {
            // Do the var here...
        }
    } break;
    case '(': {
        int level = 0;
        for (usize i = p->curr_token; i < arrlen(p->tokens); ++i) {
            switch (p->tokens[i].type) {
            case '(': ++level; break;
            case ')': --level; break;
            case TOKEN_ARROW:
            case '{': {
                if (level) goto expr_cont;

                // return NULL;
                return parse_func(p);
            } break;
            }
        }
expr_cont:
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

ANode *parse_decl(Parser *p)
{
    ADecl *a = arena_alloc(&p->node_arena, sizeof(ADecl));
    memset(a, 0, sizeof(ADecl));
    a->base.base.kind = NODE_DECL;

    a->sym = get(p).lit;
    consume(p, ':');
    if (peek(p).type == TOKEN_IDENT) {
        a->type_name = get(p).lit;
    }

    return (ANode*)a;
}

ANode *parse_assign(Parser *p, ANode *lhs)
{
    AAssign *a = arena_alloc(&p->node_arena, sizeof(AAssign));
    memset(a, 0, sizeof(AAssign));
    a->base.base.kind = NODE_ASSIGN;

    a->is_const = (peek(p).type == ':');

    if (a->is_const) consume(p, ':');
    else consume(p, '=');

    a->lhs = lhs;
    a->rhs = parse_expr(p);
    
    if (a->rhs->kind != NODE_FUNC) consume(p, ';');

    return (ANode*)a;
}

ANode *parse_expr_stmt(Parser *p)
{
    AExprStmt *es = arena_alloc(&p->node_arena, sizeof(AExprStmt));
    memset(es, 0, sizeof(es));
    es->base.base.kind = NODE_EXPR_STMT;
    
    es->expr = parse_expr(p);
    
    consume(p, ';');
    
    return es;
}

ANode *parse_return(Parser *p)
{
    consume(p, TOKEN_RETURN);
 
    AReturn *ret = arena_alloc(&p->node_arena, sizeof(AReturn));
    memset(ret, 0, sizeof(AReturn));
    ret->base.base.kind = NODE_RETURN;
    
    while(p->curr_token < arrlen(p->tokens)) {
        arrpush(ret->rets, parse_expr(p));
        if (p->curr_token < arrlen(p->tokens) && peek(p).type == ';') break;
        consume(p, ',');
    }
    
    consume(p, ';');
    
    return ret;
}

ANode *parse_stmt(Parser *p)
{
    AStmt *a = NULL;
    switch (peek(p).type) {
    case TOKEN_COMP_DIR: {
        a = (AStmt*)parse_comp_dir(p);
    } break;
    case TOKEN_IDENT: {
        if (peek2(p).type == ':' || peek2(p).type == '=') {
            ANode *lhs = parse_decl(p);
            a = (AStmt*)parse_assign(p, lhs);
            break;
        } else
        if (peek2(p).type == '(') {
            a = (AStmt*)parse_expr_stmt(p);
            break;
        } else error(p, S("Unexpected token!"));
    } break;
    case TOKEN_RETURN: {
        a = parse_return(p);
    } break;
    default: error(p, S("Unexpected token")); return (ANode*)a;
    }
    while (p->curr_token < arrlen(p->tokens) && peek(p).type == TOKEN_LABEL) {
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
    while (!p->is_err && p->curr_token < arrlen(p->tokens) && peek(p).type != '}') {
        a = parse_stmt(p);
        arrpush(scope->stmts, a);
        if (p->is_err) break;
    }

    return (ANode*)scope;
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
    case NODE_EXPR_STMT: {
        AExprStmt *ex = (AExprStmt*)a;
        printf("ExprStmt:\n");
        print_ast(ex->expr);
        printf(";\n");
    } break;
    case NODE_COMP_DIR: {
        ACompDir *cd = (ACompDir*)a;
        printf("#%.*s\n", cd->directive.len, cd->directive.data);
        for (int i = 0; i < arrlen(cd->args); ++i) {
            print_ast(cd->args[i]);
        }
        printf(";\n");
    } break;
    case NODE_DECL: {
        ADecl *decl = (ADecl*)a;
        printf("%.*s: %.*s\n", decl->sym.len, decl->sym.data, decl->type_name.len, decl->type_name.data);
    } break;
    case NODE_ASSIGN: {
        AAssign *as = (AAssign*)a;
        print_ast(as->lhs);
        printf("%c\n", as->is_const ? ':' : '=');
        print_ast(as->rhs);
    } break;
    case NODE_FUNC: {
        AFunc *fn = (AFunc*)a;
        printf("(\n");
        if (fn->args) for (int i = 0; i < arrlen(fn->args); ++i) print_ast(fn->args[i]);
        printf(") ->\n");
        if (fn->rets) for (int i = 0; i < arrlen(fn->rets); ++i) printf("%.*s\n", fn->rets[i].len, fn->rets[i].data);
        print_ast(fn->body);
    } break;
    case NODE_RETURN: {
        AReturn *ret = (AReturn*)a;
        printf("return:\n");
        if (ret->rets) for (int i = 0; i < arrlen(ret->rets); ++i) { print_ast(ret->rets[i]); printf(",\n"); }
    } break;
    case NODE_FUNC_CALL: {
        AFuncCall *fnc = (AFuncCall*)a;
        printf("CALL %.*s\n(\n", fnc->name.len, fnc->name.data);
        if (fnc->args) for (int i = 0; i < arrlen(fnc->args); ++i) {
            print_ast(fnc->args[i]);
            printf(",\n");
        }
        printf(")\n");
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
