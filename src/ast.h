#ifndef __JSON_AST_H__
#define __JSON_AST_H__

#include <stddef.h>

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
    char *value;
    ASTKind kind;
} ASTNode;


/** Construct a null node and return its pointer or NULL. */
ASTNode *ast_construct_nullnode(Token *token);

/** Construct a true/false node and return its pointer or NULL. */
ASTNode *ast_construct_boolnode(Token *token);

/** Recursively destruct entrie AST starting from *root*. */
void *ast_destruct(ASTNode *root);

/** Destruct a single AST node as well as textual values. */
void *ast_destruct_node(ASTNode *node);

/** Print a single AST node. */
void *ast_print_node(ASTNode *node);

#endif
