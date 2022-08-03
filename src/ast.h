#ifndef __JSON_AST_H__
#define __JSON_AST_H__

#include <stddef.h>

#include "json.h"
#include "token.h"


typedef enum ASTKind {
    AST_NULL,
    AST_BOOL,
    AST_STRING,
    AST_ARRAY,
    AST_OBJECT
} ASTKind;


typedef struct ASTNode {
    size_t len;
    struct ASTNode **children;
    Token *token;
    ASTKind kind;
} ASTNode;


#endif
