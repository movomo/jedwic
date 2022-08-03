#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"


#define AST_INITIAL_CAPACITY        1
#define AST_GROW_FACTOR             2


/** Construct a generic node and keep a copy of *value*. */
static ASTNode *_ast_construct(char *value, size_t capacity) {
    ASTNode *node = calloc(1, sizeof (ASTNode));
    size_t val_len = strlen(value);

    if (!node) {
        return NULL;
    }
    if (capacity == SIZE_MAX) {
        capacity = 1;
    }
    node->children = malloc(capacity * sizeof (ASTNode *));
    if (!node->children) {
        return NULL;
    }
    node->len = 0;
    node->value = malloc((val_len + 1) * sizeof (char));
    if (!node->value) {
        return NULL;
    }
    if (!memcpy(node->value, value, val_len + 1)) {
        return NULL;
    }
    return node;
}

static bool _ast_grow(ASTNode *node) {}


/** Construct a null node and return its pointer or NULL. */
ASTNode *ast_construct_nullnode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_NULL;
    return node;
}

/** Construct a true/false node and return its pointer or NULL. */
ASTNode *ast_construct_boolnode(Token *token) {
    ASTNode *node = _ast_construct(token->value, 0);

    if (!node) {
        return NULL;
    }
    node->kind = AST_BOOL;
    return node;
}

void *ast_destruct(ASTNode *root) {}

void *ast_destruct_node(ASTNode *node) {}

bool ast_append(ASTNode *parent, ASTNode *child) {}

void *ast_print(ASTNode *node) {}
