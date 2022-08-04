#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "token.h"


#define AST_INITIAL_CAPACITY        1
#define AST_GROW_FACTOR             2
#define AST_INDENT                  2


static void _ast_print_node(ASTNode *node, int depth) {
    depth *= AST_INDENT;
    for (int i = 0; i < depth; i++) {
        if (i % AST_INDENT == AST_INDENT - 1) {
            printf("|");
        } else {
            printf(" ");
        }
    }
    printf(
        "ASTNode { .kind=%d, .value=%s, .len=%llu }\n",
        node->kind,
        node->value,
        node->len
    );
}

static void *_ast_print_tree(ASTNode *root, int depth) {
    size_t i;
    _ast_print_node(root, depth);
    for (i = 0; i < root->len; i++) {
        _ast_print_tree(root->children[i], depth + 1);
    }
}

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

/** Recursively destruct entrie AST starting from *root*. */
void *ast_destruct(ASTNode *root) {
    size_t i;
    for (i = 0; i < root->len; i++) {
        ast_destruct(root->children[i]);
    }
    ast_destruct_node(root);
}

/** Destruct a single AST node as well as textual values. */
void *ast_destruct_node(ASTNode *node) {
    free(node->value);
    free(node->children);
    free(node);
}

bool ast_append(ASTNode *parent, ASTNode *child) {}

/** Print a single AST node. */
void *ast_print_node(ASTNode *node) {
    _ast_print_node(node, 0);
}

/** Print entire AST. */
void *ast_print_tree(ASTNode *root) {
    _ast_print_tree(root, 0);
}
