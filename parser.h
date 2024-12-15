#ifndef _PARSER_H
#define _PARSER_H

typedef enum ANodeKind {
    NODE_SCOPE,

    NODE_COMP_DIR,

    NODE_LIT,
} ANodeKind;

typedef struct ANode {
    ANodeKind kind;
} ANode;

typedef struct AScope {
    ANode base;

    ANode **stmts;
} AScope;

typedef struct AStmt {
    ANode base;
    string *labels;
} AStmt;

typedef struct AExpr {
    ANode base;
    string *labels;
} AExpr;

typedef struct AExprStmt {
    AStmt base;

    ANode *expr;
} AExprStmt;

typedef struct ACompDir {
    AStmt base;

    string directive;
    ANode **args;
} ACompDir;

typedef struct ADecl {
    AExpr base;

    string sym;
    string type_name;
} ADecl;

typedef struct AAssign {
    AStmt base;

    bool is_const;

    ANode *lhs;
    ANode *rhs;
} AAssign;

typedef struct AFunc {
    AExpr base;

    ANode **args;
    ANode **rets;

    ANode *body; // This needs to be a scope...
} AFunc;

typedef struct AFuncCall {
    AExpr base;

    ANode **args;
} AFuncCall;

typedef enum ALitKind {
    LIT_INT,
    LIT_UINT,
    LIT_FLOAT,
    LIT_STRING,
} ALitKind;

typedef struct ALit {
    AExpr base;

    ALitKind kind;

    union {
        ssize INT;
        usize UINT;
        double FLOAT;
        string STRING;
    } as;
} ALit;

typedef enum AOpKind {
    // Binary
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    // Unary
    OP_INV,
} AOpKind;

typedef struct AOp {
    AExpr base;

    AOpKind kind;

    ANode *lhs;
    ANode *rhs;
} AOp;

typedef struct Parser {
    Token *tokens;
    usize curr_token;

    Arena node_arena;

    bool is_err;
    string err;
} Parser;

Parser parser_from_lexer(Lexer *lex);
ANode *parser_parse_tokens(Parser *p);

void parser_free(Parser *p);

void print_ast(ANode *a);

#endif
