#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"


static ASTNode *_ast_construct(size_t capacity) {
    ASTNode *node = calloc(1, sizeof (ASTNode));

    if (!node) {
        return NULL;
    }
    if (capacity) {
        node->children = malloc(capacity * sizeof (ASTNode *));
        if (!node->children) {
            return NULL;
        }
        node->len = capacity;
    }
    return node;
}

static bool _ast_grow(ASTNode *node) {}


/** Construct a null node and return its pointer or NULL. */
ASTNode *ast_construct_nullnode(Token *token) {
    ASTNode *node = _ast_construct(0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_NULL;
    node->token = token;
    return node;
}

/** Construct a true/false node and return its pointer or NULL. */
ASTNode *ast_construct_boolnode(Token *token) {
    ASTNode *node = _ast_construct(0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_BOOL;
    node->token = token;
    return node;
}

void *ast_destruct(ASTNode *root) {}

void *ast_destruct_node(ASTNode *node) {}

bool ast_append(ASTNode *parent, ASTNode *child) {}

void *ast_print(ASTNode *node) {}
