#ifndef _PARSER_H
#define _PARSER_H

typedef enum AType {
    AST_LABEL,
    
    AST_SCOPE,
    
    AST_STMT_COMP_DIR,

    AST_STMT_CONST_ASSIGN,
    AST_STMT_ASSIGN,
    
    AST_STMT_DECL,
    
    AST_STMT_EXPR,
    
    AST_EXPR_FUNC,
    AST_EXPR_FUNC_CALL,
    AST_EXPR_FUNC_CALL_ARG,
    AST_EXPR_LIT,
    AST_EXPR_BINOP,
    AST_EXPR_UNOP,
} AType;

typedef struct ABase {
    AType type;
} ABase;

typedef struct ALabel {
    ABase base;
    
    String label;
} ALabel;

typedef struct Sym {
    String ident;
    usize type; // Index into the type table; or a hash of the type.
} Sym;

typedef struct AScope {
    ABase base;
    
    Sym *symTable; // Hash map?
} AScope;

typedef struct AStmtCompDir {
    ABase base;
    
    String directive;
    
    ABase *arg;
} AStmtCompDir;

typedef struct AStmtConstAssign {
    ABase base;
    
    ABase *target;
    ABase *value;
} AStmtConstAssign;

typedef struct AStmtAssign {
    ABase base;
    
    ABase *target;
    ABase *value;
} AStmtAssign;

typedef struct AStmtDecl {
    ABase base;
    
    String ident;
    
    ABase *target;
    ABase *value;
} AStmtDecl;

typedef struct ParserError {
    bool isErr;
    String err;
} ParseError;

typedef struct Package {
    String packageName;
    ABase *module;
    
    ParseError err;
} Package;

Package ParsePackage(Token *tokens);

#endif