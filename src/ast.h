#ifndef __JSON_AST_H__
#define __JSON_AST_H__

#include <stddef.h>

#include "token.h"


typedef enum ASTKind {
    AST_NULL,
    AST_BOOL,
    AST_NUMBER,
    AST_STRING,
    AST_ARRAY,
    AST_OBJECT,
    AST_KEY
} ASTKind;

typedef struct ASTNode {
    size_t len;
    size_t cap;
    struct ASTNode **children;
    char *value;
    ASTKind kind;
} ASTNode;


/** Construct a null node and return its pointer or NULL. */
ASTNode *ast_construct_nullnode(Token *token);

/** Construct a true/false node and return its pointer or NULL. */
ASTNode *ast_construct_boolnode(Token *token);

/** Construct a number node and return its pointer or NULL. */
ASTNode *ast_construct_numbernode(Token *token);

/** Construct a string node and return its pointer or NULL. */
ASTNode *ast_construct_stringnode(Token *token);

/** Construct an array node and return its pointer or NULL. */
ASTNode *ast_construct_arraynode(Token *token);

/** Construct a key node and return its pointer or NULL. */
ASTNode *ast_construct_keynode(Token *token);

/** Construct an object node and return its pointer or NULL. */
ASTNode *ast_construct_objectnode(Token *token);

/** Append a child node to the parent node and report success as bool. */
bool ast_append(ASTNode *parent, ASTNode *child);

/** Recursively destruct entrie AST starting from *root*. */
void *ast_destruct(ASTNode *root);

/** Destruct a single AST node as well as textual values. */
void *ast_destruct_node(ASTNode *node);

/** Print a single AST node. */
void *ast_print_node(ASTNode *node);

/** Print entire AST. */
void *ast_print_tree(ASTNode *root);

#endif
